// hdf5: a file is written, closed, reopened and read back through the C
// API, the high-level API (H5LT) and the C++ API, with known answers:
// chunked integer data through the DEFLATE (zlib) and SZIP (libaec)
// filters, a hyperslab, a compound type, attributes, a group tree, the file
// signature on disk, an in-memory (core driver) file, and the _Float16
// conversion this port enables by hand in its cross build (IEEE 754
// binary16 bit patterns). The tools h5ls, h5dump, h5repack and h5diff are
// run on the file the library wrote.
#include "common.h"
#include <hdf5.h>
#include <hdf5_hl.h>
#include <H5Cpp.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

namespace {
const int NX = 64, NY = 48;
int cell(int i, int j) { return (i * 7 + j * 3) % 11; }   // compressible, known

struct Rec { int32_t id; double w; char tag[8]; };

// A chunked NX x NY int dataset through one filter; returns its stored size.
hsize_t write_filtered(hid_t file, const char *name, bool szip, const std::vector<int> &data) {
  hsize_t dims[2] = {NX, NY}, chunk[2] = {16, 16};
  hid_t space = H5Screate_simple(2, dims, NULL);
  hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(dcpl, 2, chunk);
  if (szip) H5Pset_szip(dcpl, H5_SZIP_NN_OPTION_MASK, 16);
  else { H5Pset_shuffle(dcpl); H5Pset_deflate(dcpl, 6); }
  hid_t ds = H5Dcreate2(file, name, H5T_STD_I32LE, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
  hsize_t stored = 0;
  if (ds >= 0) {
    if (H5Dwrite(ds, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data()) >= 0) {
      H5Dflush(ds);
      stored = H5Dget_storage_size(ds);
    }
    H5Dclose(ds);
  }
  H5Pclose(dcpl); H5Sclose(space);
  return stored;
}

bool read_equals(hid_t file, const char *name, const std::vector<int> &want) {
  std::vector<int> got(want.size(), -1);
  hid_t ds = H5Dopen2(file, name, H5P_DEFAULT);
  if (ds < 0) return false;
  bool ok = H5Dread(ds, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, got.data()) >= 0 && got == want;
  H5Dclose(ds);
  return ok;
}

// Run one of the HDF5 tools (the installed one, else from PATH); its output
// goes to `out`, its exit status is returned (-1: could not be run).
int run_tool(const char *tool, const std::string &args, std::string &out) {
  std::string prog = std::string("/usr/local/bin/") + tool;
  if (access(prog.c_str(), X_OK) != 0) prog = tool;
  std::string cmd = prog + " " + args + " 2>&1";
  out.clear();
  FILE *p = popen(cmd.c_str(), "r");
  if (!p) return -1;
  char buf[512]; size_t n;
  while ((n = fread(buf, 1, sizeof buf, p)) > 0) out.append(buf, n);
  int st = pclose(p);
  int rc = (st != -1 && WIFEXITED(st)) ? WEXITSTATUS(st) : -1;
  if (rc != 0) printf("  info: %s -> exit %d: %.300s\n", cmd.c_str(), rc, out.c_str());
  return rc;
}

// Is there an output line that contains both strings?
bool has_line(const std::string &out, const char *a, const char *b) {
  size_t pos = 0;
  while (pos < out.size()) {
    size_t e = out.find('\n', pos); if (e == std::string::npos) e = out.size();
    std::string line = out.substr(pos, e - pos);
    if (line.find(a) != std::string::npos && line.find(b) != std::string::npos) return true;
    pos = e + 1;
  }
  return false;
}
}  // namespace

int main() {
  QT_BEGIN("hdf5");
  H5Eset_auto2(H5E_DEFAULT, NULL, NULL);   // failures are checked, not printed

  unsigned maj = 0, min = 0, rel = 0;
  QT_CHECK(H5open() >= 0 && H5get_libversion(&maj, &min, &rel) >= 0 && maj == H5_VERS_MAJOR &&
           min == H5_VERS_MINOR && rel == H5_VERS_RELEASE, "library version matches the headers");
  QT_CHECK(maj == 1 && min == 14, "this is HDF5 1.14");

  unsigned flt = 0;
  QT_CHECK(H5Zfilter_avail(H5Z_FILTER_DEFLATE) > 0 && H5Zget_filter_info(H5Z_FILTER_DEFLATE, &flt) >= 0 &&
           (flt & H5Z_FILTER_CONFIG_ENCODE_ENABLED) && (flt & H5Z_FILTER_CONFIG_DECODE_ENABLED),
           "DEFLATE filter encodes and decodes");
  flt = 0;
  QT_CHECK(H5Zfilter_avail(H5Z_FILTER_SZIP) > 0 && H5Zget_filter_info(H5Z_FILTER_SZIP, &flt) >= 0 &&
           (flt & H5Z_FILTER_CONFIG_ENCODE_ENABLED) && (flt & H5Z_FILTER_CONFIG_DECODE_ENABLED),
           "SZIP filter encodes and decodes");

  char path[512];
  snprintf(path, sizeof path, "%s/qnx-hdf5-%d.h5", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());

  std::vector<int> data(NX * NY);
  long long sum = 0;
  for (int i = 0; i < NX; i++) for (int j = 0; j < NY; j++) { data[i * NY + j] = cell(i, j); sum += cell(i, j); }

  // ---- write (C API) ----
  hid_t file = H5Fcreate(path, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  QT_CHECK(file >= 0, "H5Fcreate with the default file access (file locking: best effort)");
  if (file < 0) {
    // Tell a locking problem of the file system apart from everything else.
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_file_locking(fapl, 0, 1);
    file = H5Fcreate(path, H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    printf("  note: H5Fcreate without file locking %s\n", file >= 0 ? "works" : "fails too");
    H5Pclose(fapl);
  }
  if (file < 0) { printf("FAIL hdf5 (cannot create %s)\n", path); return 1; }

  hid_t lcpl = H5Pcreate(H5P_LINK_CREATE);
  H5Pset_create_intermediate_group(lcpl, 1);
  hid_t grp = H5Gcreate2(file, "/sensors/lidar/front", lcpl, H5P_DEFAULT, H5P_DEFAULT);
  QT_CHECK(grp >= 0, "nested groups created");
  H5Pclose(lcpl);

  const hsize_t raw = (hsize_t)NX * NY * 4;
  hsize_t zs = write_filtered(file, "/sensors/grid_deflate", false, data);
  hsize_t ss = write_filtered(file, "/sensors/grid_szip", true, data);
  QT_CHECK(zs > 0 && zs < raw / 2, "DEFLATE+shuffle dataset is stored in less than half its raw size");
  QT_CHECK(ss > 0 && ss < raw, "SZIP dataset is stored smaller than raw");

  // attributes: a scalar double and a fixed-length string on the group
  {
    hid_t sp = H5Screate(H5S_SCALAR);
    double rate = 12.5;
    hid_t a = H5Acreate2(grp, "rate_hz", H5T_IEEE_F64LE, sp, H5P_DEFAULT, H5P_DEFAULT);
    bool ok = a >= 0 && H5Awrite(a, H5T_NATIVE_DOUBLE, &rate) >= 0;
    if (a >= 0) H5Aclose(a);
    hid_t st = H5Tcopy(H5T_C_S1); H5Tset_size(st, 16);
    char unit[16] = "millimetre";
    a = H5Acreate2(grp, "unit", st, sp, H5P_DEFAULT, H5P_DEFAULT);
    ok = ok && a >= 0 && H5Awrite(a, st, unit) >= 0;
    if (a >= 0) H5Aclose(a);
    H5Tclose(st); H5Sclose(sp);
    QT_CHECK(ok, "attributes written");
  }

  // compound type
  hid_t rt = H5Tcreate(H5T_COMPOUND, sizeof(Rec));
  hid_t tag_t = H5Tcopy(H5T_C_S1); H5Tset_size(tag_t, 8);
  H5Tinsert(rt, "id", HOFFSET(Rec, id), H5T_NATIVE_INT32);
  H5Tinsert(rt, "w", HOFFSET(Rec, w), H5T_NATIVE_DOUBLE);
  H5Tinsert(rt, "tag", HOFFSET(Rec, tag), tag_t);
  {
    Rec recs[3] = {{1, 0.5, "alpha"}, {-7, 2.25, "beta"}, {1000000, -1e300, "gamma"}};
    hsize_t n = 3;
    hid_t sp = H5Screate_simple(1, &n, NULL);
    hid_t ds = H5Dcreate2(grp, "records", rt, sp, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    QT_CHECK(ds >= 0 && H5Dwrite(ds, rt, H5S_ALL, H5S_ALL, H5P_DEFAULT, recs) >= 0, "compound dataset written");
    if (ds >= 0) H5Dclose(ds);
    H5Sclose(sp);
  }

  // high-level API
  {
    double v[6] = {1.0, 2.5, -3.0, 4.25, 1e-3, 6.0};
    hsize_t d[2] = {2, 3};
    QT_CHECK(H5LTmake_dataset_double(file, "/hl_matrix", 2, d, v) >= 0 &&
             H5LTset_attribute_string(file, "/hl_matrix", "frame", "base_link") >= 0,
             "H5LT dataset and string attribute written");
  }
  H5Gclose(grp);
  QT_CHECK(H5Fclose(file) >= 0, "file closed");

  // ---- the file on disk ----
  {
    static const unsigned char sig[8] = {0x89, 'H', 'D', 'F', '\r', '\n', 0x1a, '\n'};
    unsigned char head[8] = {0};
    FILE *f = fopen(path, "rb");
    bool ok = f && fread(head, 1, 8, f) == 8 && memcmp(head, sig, 8) == 0;
    if (f) fclose(f);
    QT_CHECK(ok, "file starts with the HDF5 signature");
    QT_CHECK(H5Fis_accessible(path, H5P_DEFAULT) > 0, "H5Fis_accessible recognises it");
  }

  // ---- read back (C API) ----
  file = H5Fopen(path, H5F_ACC_RDONLY, H5P_DEFAULT);
  QT_CHECK(file >= 0, "file reopened read-only");
  if (file >= 0) {
    QT_CHECK(read_equals(file, "/sensors/grid_deflate", data), "DEFLATE dataset reads back identical");
    QT_CHECK(read_equals(file, "/sensors/grid_szip", data), "SZIP dataset reads back identical");

    // a 5 x 4 hyperslab starting at (10, 20), every second column
    {
      hid_t ds = H5Dopen2(file, "/sensors/grid_deflate", H5P_DEFAULT);
      hid_t fs = H5Dget_space(ds);
      hsize_t start[2] = {10, 20}, stride[2] = {1, 2}, count[2] = {5, 4}, md[2] = {5, 4};
      hid_t ms = H5Screate_simple(2, md, NULL);
      int part[20]; bool ok = H5Sselect_hyperslab(fs, H5S_SELECT_SET, start, stride, count, NULL) >= 0 &&
                              H5Sget_select_npoints(fs) == 20 &&
                              H5Dread(ds, H5T_NATIVE_INT, ms, fs, H5P_DEFAULT, part) >= 0;
      for (int i = 0; ok && i < 5; i++) for (int j = 0; j < 4; j++) ok = ok && part[i * 4 + j] == cell(10 + i, 20 + 2 * j);
      QT_CHECK(ok, "strided hyperslab holds the expected cells");
      // type conversion on read: int32 on disk -> int64 in memory
      std::vector<long long> wide(NX * NY); long long s = 0;
      ok = H5Dread(ds, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, wide.data()) >= 0;
      for (long long x : wide) s += x;
      QT_CHECK(ok && s == sum, "read as 64-bit integers, sum matches");
      hid_t dcpl = H5Dget_create_plist(ds);
      QT_CHECK(H5Pget_nfilters(dcpl) == 2 && H5Pget_layout(dcpl) == H5D_CHUNKED, "dataset reports chunked layout with 2 filters");
      H5Pclose(dcpl); H5Sclose(ms); H5Sclose(fs); H5Dclose(ds);
    }

    {
      double rate = 0; char unit[16] = {0};
      hid_t a = H5Aopen_by_name(file, "/sensors/lidar/front", "rate_hz", H5P_DEFAULT, H5P_DEFAULT);
      bool ok = a >= 0 && H5Aread(a, H5T_NATIVE_DOUBLE, &rate) >= 0 && rate == 12.5;
      if (a >= 0) H5Aclose(a);
      hid_t st = H5Tcopy(H5T_C_S1); H5Tset_size(st, 16);
      a = H5Aopen_by_name(file, "/sensors/lidar/front", "unit", H5P_DEFAULT, H5P_DEFAULT);
      ok = ok && a >= 0 && H5Aread(a, st, unit) >= 0 && strcmp(unit, "millimetre") == 0;
      if (a >= 0) H5Aclose(a);
      H5Tclose(st);
      QT_CHECK(ok, "attributes read back: 12.5 and \"millimetre\"");
    }

    {
      Rec got[3]; memset(got, 0, sizeof got);
      hid_t ds = H5Dopen2(file, "/sensors/lidar/front/records", H5P_DEFAULT);
      bool ok = ds >= 0 && H5Dread(ds, rt, H5S_ALL, H5S_ALL, H5P_DEFAULT, got) >= 0 &&
                got[0].id == 1 && got[0].w == 0.5 && strcmp(got[0].tag, "alpha") == 0 &&
                got[1].id == -7 && got[1].w == 2.25 && strcmp(got[1].tag, "beta") == 0 &&
                got[2].id == 1000000 && got[2].w == -1e300 && strcmp(got[2].tag, "gamma") == 0;
      if (ds >= 0) {
        hid_t ft = H5Dget_type(ds);
        ok = ok && H5Tget_class(ft) == H5T_COMPOUND && H5Tget_nmembers(ft) == 3;
        H5Tclose(ft); H5Dclose(ds);
      }
      QT_CHECK(ok, "compound records read back");
    }

    {
      H5G_info_t gi; memset(&gi, 0, sizeof gi);
      QT_CHECK(H5Gget_info_by_name(file, "/sensors", &gi, H5P_DEFAULT) >= 0 && gi.nlinks == 3,
               "/sensors has 3 links");
      QT_CHECK(H5Lexists(file, "/sensors/lidar/front", H5P_DEFAULT) > 0 &&
               H5Lexists(file, "/sensors/radar", H5P_DEFAULT) == 0, "H5Lexists tells present from absent");
      QT_CHECK(H5Dopen2(file, "/no/such/dataset", H5P_DEFAULT) < 0, "opening a missing dataset fails");
    }

    {
      int rank = 0; hsize_t d[2] = {0, 0}; double v[6] = {0}; char frame[32] = {0};
      bool ok = H5LTget_dataset_ndims(file, "/hl_matrix", &rank) >= 0 && rank == 2 &&
                H5LTget_dataset_info(file, "/hl_matrix", d, NULL, NULL) >= 0 && d[0] == 2 && d[1] == 3 &&
                H5LTread_dataset_double(file, "/hl_matrix", v) >= 0 &&
                v[0] == 1.0 && v[1] == 2.5 && v[2] == -3.0 && v[3] == 4.25 && v[4] == 1e-3 && v[5] == 6.0 &&
                H5LTget_attribute_string(file, "/hl_matrix", "frame", frame) >= 0 && strcmp(frame, "base_link") == 0;
      QT_CHECK(ok, "H5LT reads the 2x3 matrix and its attribute");
    }
    H5Fclose(file);
  }
  H5Tclose(tag_t); H5Tclose(rt);

  // ---- C++ API on the same file ----
  try {
    H5::Exception::dontPrint();
    H5::H5File f(path, H5F_ACC_RDWR);
    H5::DataSet ds = f.openDataSet("/sensors/grid_szip");
    hsize_t d[2] = {0, 0};
    int rank = ds.getSpace().getSimpleExtentDims(d);
    std::vector<int> got(NX * NY, -1);
    ds.read(got.data(), H5::PredType::NATIVE_INT);
    QT_CHECK(rank == 2 && d[0] == (hsize_t)NX && d[1] == (hsize_t)NY && got == data, "C++: dataset dimensions and contents");
    // an extendible 1-D dataset, grown and appended to
    hsize_t n0 = 4, nmax = H5S_UNLIMITED, ch = 8;
    H5::DSetCreatPropList cp; cp.setChunk(1, &ch);
    H5::DataSet e = f.createDataSet("/series", H5::PredType::IEEE_F32LE, H5::DataSpace(1, &n0, &nmax), cp);
    float a[4] = {1, 2, 3, 4}, b[3] = {5, 6, 7};
    e.write(a, H5::PredType::NATIVE_FLOAT);
    hsize_t n1 = 7, off = 4, cnt = 3;
    e.extend(&n1);
    H5::DataSpace fs = e.getSpace(); fs.selectHyperslab(H5S_SELECT_SET, &cnt, &off);
    e.write(b, H5::PredType::NATIVE_FLOAT, H5::DataSpace(1, &cnt), fs);
    double all[7] = {0}; e.read(all, H5::PredType::NATIVE_DOUBLE);
    double s = 0; for (double x : all) s += x;
    QT_CHECK(e.getSpace().getSimpleExtentNpoints() == 7 && s == 28.0 && all[6] == 7.0, "C++: extendible dataset grown to 7 values, sum 28");
    bool threw = false;
    try { f.openGroup("/absent"); } catch (const H5::Exception &) { threw = true; }
    QT_CHECK(threw, "C++: opening a missing group throws H5::Exception");
    QT_CHECK(f.getObjCount(H5F_OBJ_DATASET) >= 1, "C++: open dataset handles are counted");
  } catch (const H5::Exception &ex) {
    printf("  C++ exception: %s\n", ex.getCDetailMsg());
    QT_CHECK(false, "C++ API without unexpected exceptions");
  }

  // ---- the command line tools on the same file ----
  {
    std::string out;
    int rc = run_tool("h5ls", std::string("-r '") + path + "'", out);
    QT_CHECK(rc == 0 && has_line(out, "/sensors/grid_deflate", "Dataset {64, 48}") &&
             has_line(out, "/sensors/lidar/front/records", "Dataset {3}") && has_line(out, "/hl_matrix", "Dataset {2, 3}") &&
             has_line(out, "/series", "Dataset {7/Inf}"), "h5ls -r lists the datasets with their dimensions");
    rc = run_tool("h5dump", std::string("-d /hl_matrix '") + path + "'", out);
    QT_CHECK(rc == 0 && out.find("(0,0): 1, 2.5, -3,") != std::string::npos && out.find("(1,0): 4.25, 0.001, 6") != std::string::npos &&
             out.find("\"base_link\"") != std::string::npos, "h5dump prints the matrix values and the attribute");
    std::string packed = std::string(path) + ".repack";
    rc = run_tool("h5repack", std::string("-f GZIP=9 '") + path + "' '" + packed + "'", out);
    QT_CHECK(rc == 0 && access(packed.c_str(), R_OK) == 0, "h5repack rewrites the file with GZIP=9");
    rc = run_tool("h5diff", std::string("'") + path + "' '" + packed + "'", out);
    QT_CHECK(rc == 0, "h5diff finds no difference between the file and its repacked copy");
    // change one value in the copy: h5diff must report it (exit status 1)
    hid_t f2 = H5Fopen(packed.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    int v[4] = {1, 2, 3, 4}; hsize_t n = 4;
    bool ok = f2 >= 0 && H5LTmake_dataset_int(f2, "/extra", 1, &n, v) >= 0;
    double m[6] = {1.0, 2.5, -3.0, 4.25, 1e-3, 7.0};
    hid_t d2 = f2 >= 0 ? H5Dopen2(f2, "/hl_matrix", H5P_DEFAULT) : -1;
    ok = ok && d2 >= 0 && H5Dwrite(d2, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, m) >= 0;
    if (d2 >= 0) H5Dclose(d2);
    if (f2 >= 0) H5Fclose(f2);
    rc = run_tool("h5diff", std::string("'") + path + "' '" + packed + "' /hl_matrix", out);
    QT_CHECK(ok && rc == 1 && out.find("1 differences found") != std::string::npos, "h5diff reports the one changed value (exit status 1)");
    unlink(packed.c_str());
  }
  QT_CHECK(unlink(path) == 0, "file removed");

  // ---- in-memory file (core driver, nothing written to disk) ----
  {
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_core(fapl, 1 << 16, 0);
    hid_t mf = H5Fcreate("qnx-hdf5-core-only.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    int v[4] = {3, 1, 4, 1}, r[4] = {0}; hsize_t n = 4;
    bool ok = mf >= 0 && H5LTmake_dataset_int(mf, "pi", 1, &n, v) >= 0 && H5Fflush(mf, H5F_SCOPE_GLOBAL) >= 0 &&
              H5LTread_dataset_int(mf, "pi", r) >= 0 && memcmp(v, r, sizeof v) == 0;
    ssize_t img = mf >= 0 ? H5Fget_file_image(mf, NULL, 0) : -1;
    if (mf >= 0) H5Fclose(mf);
    H5Pclose(fapl);
    QT_CHECK(ok && img > 8, "core-driver file holds a dataset and has a file image");
    QT_CHECK(access("qnx-hdf5-core-only.h5", F_OK) != 0, "core-driver file without backing store left nothing on disk");
  }

  // ---- _Float16: the conversions the recipe answers for the cross build ----
  {
    // IEEE 754 binary16: 1.5 = 0x3E00, -2.25 = 0xC080, 65504 (largest) = 0x7BFF, 2^-24 (smallest subnormal) = 0x0001
    double in[4] = {1.5, -2.25, 65504.0, 5.9604644775390625e-8};
    union { double d[4]; unsigned char b[32]; } buf;
    memcpy(buf.d, in, sizeof in);
    bool ok = H5Tconvert(H5T_NATIVE_DOUBLE, H5T_IEEE_F16LE, 4, buf.b, NULL, H5P_DEFAULT) >= 0;
    static const unsigned char want[8] = {0x00, 0x3E, 0x80, 0xC0, 0xFF, 0x7B, 0x01, 0x00};
    QT_CHECK(ok && memcmp(buf.b, want, 8) == 0, "double -> IEEE half: 1.5, -2.25, 65504, 2^-24 have the binary16 bit patterns");
    ok = H5Tconvert(H5T_IEEE_F16LE, H5T_NATIVE_DOUBLE, 4, buf.b, NULL, H5P_DEFAULT) >= 0;
    QT_CHECK(ok && memcmp(buf.d, in, sizeof in) == 0, "IEEE half -> double restores the values exactly");
#ifdef H5_HAVE__FLOAT16
    QT_CHECK(H5Tget_size(H5T_NATIVE_FLOAT16) == 2 && H5Tequal(H5T_NATIVE_FLOAT16, H5T_IEEE_F16LE) > 0,
             "H5T_NATIVE_FLOAT16 is the 2-byte little-endian IEEE type");
    // the compiler's own _Float16 conversions (hard conversion path in H5Tconv)
    union { long double ld[4]; int i[4]; unsigned char b[64]; } u;
    u.ld[0] = 1.5L; u.ld[1] = -2.25L; u.ld[2] = 65504.0L; u.ld[3] = 0.333251953125L;   // all exact in binary16
    ok = H5Tconvert(H5T_NATIVE_LDOUBLE, H5T_NATIVE_FLOAT16, 4, u.b, NULL, H5P_DEFAULT) >= 0;
    static const unsigned char want2[8] = {0x00, 0x3E, 0x80, 0xC0, 0xFF, 0x7B, 0x55, 0x35};
    QT_CHECK(ok && memcmp(u.b, want2, 8) == 0, "long double -> _Float16 gives the binary16 bit patterns");
    ok = H5Tconvert(H5T_NATIVE_FLOAT16, H5T_NATIVE_INT, 3, u.b, NULL, H5P_DEFAULT) >= 0;
    QT_CHECK(ok && u.i[0] == 1 && u.i[1] == -2 && u.i[2] == 65504, "_Float16 -> int truncates to 1, -2, 65504");
#else
    QT_CHECK(false, "H5_HAVE__FLOAT16 is defined (the recipe enables it)");
#endif
  }

  QT_CHECK(H5close() >= 0, "H5close");
  QT_END();
}
