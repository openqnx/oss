/* ONNX: build a model in code (Add with broadcasting, then MatMul), serialize
 * it to bytes and a file in /tmp, parse both back, run the checker and
 * shape inference (expect a [2,4] float output), and check that a broken
 * model is rejected by the checker. */
#include "common.h"
#include <onnx/onnx_pb.h>
#include <onnx/checker.h>
#include <onnx/defs/schema.h>
#include <onnx/shape_inference/implementation.h>
#include <fstream>
#include <string>
#include <unistd.h>
using namespace onnx;
static void add_input(GraphProto *g, const char *name, std::initializer_list<int64_t> dims) {
  ValueInfoProto *v = g->add_input();
  v->set_name(name);
  TypeProto_Tensor *t = v->mutable_type()->mutable_tensor_type();
  t->set_elem_type(TensorProto_DataType_FLOAT);
  for (int64_t d : dims) t->mutable_shape()->add_dim()->set_dim_value(d);
}
static ModelProto build(bool broken) {
  ModelProto m;
  m.set_ir_version(IR_VERSION);
  m.set_producer_name("qnx-port-test");
  OperatorSetIdProto *op = m.add_opset_import();
  op->set_domain("");
  op->set_version(17);
  GraphProto *g = m.mutable_graph();
  g->set_name("add_matmul");
  add_input(g, "A", {2, 3});
  add_input(g, "B", {3});
  add_input(g, "C", {3, 4});
  NodeProto *n = g->add_node();
  n->set_op_type("Add"); n->set_name("add0"); n->add_input("A");
  if (!broken) n->add_input("B");
  n->add_output("S");
  NodeProto *mm = g->add_node();
  mm->set_op_type("MatMul"); mm->set_name("mm0"); mm->add_input("S"); mm->add_input("C"); mm->add_output("Y");
  ValueInfoProto *y = g->add_output();
  y->set_name("Y");
  y->mutable_type()->mutable_tensor_type()->set_elem_type(TensorProto_DataType_FLOAT);
  /* The checker requires a shape on main-graph outputs: rank 2, dimensions
     left unknown for shape inference to fill in. */
  TensorShapeProto *ys = y->mutable_type()->mutable_tensor_type()->mutable_shape();
  ys->add_dim(); ys->add_dim();
  return m;
}
int main() {
  QT_BEGIN("onnx");
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  printf("  info IR version %d\n", (int)IR_VERSION);
  const OpSchema *s = OpSchemaRegistry::Schema("Add", 17, "");
  QT_CHECK(s != nullptr, "operator schema registry knows Add-17");
  if (s) QT_CHECK(s->min_input() == 2 && s->max_input() == 2 && s->min_output() == 1, "Add has 2 inputs, 1 output");
  QT_CHECK(OpSchemaRegistry::Schema("Conv", 17, "") != nullptr, "Conv-17 schema registered");
  ModelProto m = build(false);
  std::string bytes;
  QT_CHECK(m.SerializeToString(&bytes) && bytes.size() > 50, "ModelProto serializes");
  printf("  info serialized model: %zu bytes\n", bytes.size());
  ModelProto back;
  QT_CHECK(back.ParseFromString(bytes), "ModelProto parses back");
  QT_CHECK(back.graph().node_size() == 2 && back.graph().node(1).op_type() == "MatMul" &&
           back.producer_name() == "qnx-port-test", "parsed model has the same nodes");
  bool ok = true;
  try { checker::check_model(back); } catch (const std::exception &e) { ok = false; printf("  info checker: %s\n", e.what()); }
  QT_CHECK(ok, "checker accepts the valid model");
  ok = true;
  try { shape_inference::InferShapes(back); } catch (const std::exception &e) { ok = false; printf("  info shape inference: %s\n", e.what()); }
  QT_CHECK(ok, "shape inference runs");
  const TypeProto_Tensor &yt = back.graph().output(0).type().tensor_type();
  int nd = yt.has_shape() ? yt.shape().dim_size() : -1;
  printf("  info inferred Y rank %d", nd);
  for (int i = 0; i < nd; i++) printf(" %lld", (long long)yt.shape().dim(i).dim_value());
  printf("\n");
  QT_CHECK(nd == 2 && yt.shape().dim(0).dim_value() == 2 && yt.shape().dim(1).dim_value() == 4, "Y inferred as [2,4]");
  bool sinfo = false;
  for (const auto &vi : back.graph().value_info())
    if (vi.name() == "S" && vi.type().tensor_type().shape().dim_size() == 2 && vi.type().tensor_type().shape().dim(1).dim_value() == 3) sinfo = true;
  QT_CHECK(sinfo, "intermediate S inferred as [2,3] (broadcast)");

  char path[64]; snprintf(path, sizeof path, "/tmp/qnxport-onnx-%d.onnx", (int)getpid());
  { std::ofstream f(path, std::ios::binary); m.SerializeToOstream(&f); }
  ok = true;
  try { checker::check_model(std::string(path), true); } catch (const std::exception &e) { ok = false; printf("  info checker(file): %s\n", e.what()); }
  QT_CHECK(ok, "checker (full_check) accepts the model from a file");
  unlink(path);

  ModelProto bad = build(true);
  bool threw = false;
  try { checker::check_model(bad); } catch (const checker::ValidationError &e) { threw = true; printf("  info rejected: %.100s\n", e.what()); }
  QT_CHECK(threw, "checker rejects Add with one input");
  QT_END();
}
