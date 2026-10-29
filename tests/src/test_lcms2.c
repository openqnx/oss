/* Little-CMS: build sRGB and Lab profiles and transform colours between them. */
#include "common.h"
#include <lcms2.h>
int main(void) {
  QT_BEGIN("lcms2");
  printf("  info lcms %d\n", cmsGetEncodedCMMversion());
  cmsHPROFILE srgb = cmsCreate_sRGBProfile();
  cmsHPROFILE lab = cmsCreateLab4Profile(NULL);
  QT_CHECK(srgb != NULL && lab != NULL, "create sRGB and Lab profiles");
  cmsHTRANSFORM t = cmsCreateTransform(srgb, TYPE_RGB_8, lab, TYPE_Lab_DBL,
                                       INTENT_RELATIVE_COLORIMETRIC, 0);
  QT_CHECK(t != NULL, "cmsCreateTransform sRGB -> Lab");
  unsigned char white[3] = { 255, 255, 255 };
  cmsCIELab out;
  cmsDoTransform(t, white, &out, 1);
  QT_CHECK(out.L > 99.0 && out.L <= 100.5, "white maps to L* ~ 100");
  QT_CHECK(out.a > -1.5 && out.a < 1.5 && out.b > -1.5 && out.b < 1.5,
           "white is neutral in a*/b*");
  unsigned char black[3] = { 0, 0, 0 };
  cmsDoTransform(t, black, &out, 1);
  QT_CHECK(out.L < 1.0, "black maps to L* ~ 0");
  cmsDeleteTransform(t);
  cmsHTRANSFORM back = cmsCreateTransform(lab, TYPE_Lab_DBL, srgb, TYPE_RGB_8,
                                          INTENT_RELATIVE_COLORIMETRIC, 0);
  QT_CHECK(back != NULL, "reverse transform");
  cmsCIELab red_lab = { 53.24, 80.09, 67.20 };   /* sRGB pure red */
  unsigned char rgb[3];
  cmsDoTransform(back, &red_lab, rgb, 1);
  QT_CHECK(rgb[0] > 240 && rgb[1] < 30 && rgb[2] < 30, "Lab red maps back to sRGB red");
  cmsDeleteTransform(back);
  cmsCloseProfile(srgb); cmsCloseProfile(lab);
  QT_END();
}
