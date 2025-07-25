/*
 * PROJECT:     ReactOS API tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Tests for log10
 * COPYRIGHT:   Copyright 2021 Timo Kreuzer <timo.kreuzer@reactos.org>
 */

#if !defined(_CRTBLD) && !defined(_M_IX86)
#define _CRTBLD // we don't want inline log10!
#endif
#include "math_helpers.h"

#ifdef _MSC_VER
#pragma function(log10)
#endif

#if !defined(_M_IX86)
#define HAS_LOG10F
#elif (defined(TEST_UCRTBASE) || defined(TEST_STATIC_CRT))
#define HAS_LIBM_SSE2
#endif


// These are expected to match exactly
static TESTENTRY_DBL s_log10_exact_tests[] =
{
    { 0x0000000000000000 /*  0.000000 */, 0xfff0000000000000 /* -1.#INF00 */ },
    { 0x8000000000000000 /* -0.000000 */, 0xfff0000000000000 /* -1.#INF00 */ },
    { 0x7ff0000000000000 /*  1.#INF00 */, 0x7ff0000000000000 /*  1.#INF00 */ },
    { 0x7ff0000000000001 /*  1.#SNAN0 */, 0x7ff8000000000001 /*  1.#QNAN0 */ },
    { 0x7ff7ffffffffffff /*  1.#SNAN0 */, 0x7fffffffffffffff /*  1.#QNAN0 */ },
    { 0x7ff8000000000000 /*  1.#QNAN0 */, 0x7ff8000000000000 /*  1.#QNAN0 */ },
    { 0x7ff8000000000001 /*  1.#QNAN0 */, 0x7ff8000000000001 /*  1.#QNAN0 */ },
    { 0x7fffffffffffffff /*  1.#QNAN0 */, 0x7fffffffffffffff /*  1.#QNAN0 */ },
    { 0xfff0000000000000 /* -1.#INF00 */, 0xfff8000000000000 /* -1.#IND00 */ },
    { 0xfff0000000000001 /* -1.#SNAN0 */, 0xfff8000000000001 /* -1.#QNAN0 */ },
    { 0xfff7ffffffffffff /* -1.#SNAN0 */, 0xffffffffffffffff /* -1.#QNAN0 */ },
    { 0xfff8000000000000 /* -1.#IND00 */, 0xfff8000000000000 /* -1.#IND00 */ },
    { 0xfff8000000000001 /* -1.#QNAN0 */, 0xfff8000000000001 /* -1.#QNAN0 */ },
    { 0xffffffffffffffff /* -1.#QNAN0 */, 0xffffffffffffffff /* -1.#QNAN0 */ },
};

void Test_log10_exact(void)
{
    for (int i = 0; i < _countof(s_log10_exact_tests); i++)
    {
        double x = u64_to_dbl(s_log10_exact_tests[i].x);
        double z = log10(x);
        ok_eq_dbl_exact("log10", s_log10_exact_tests[i].x, z, s_log10_exact_tests[i].result);
    }
}

// This table is autogenerated by `python gen_math_tests.py log10`
static TESTENTRY_DBL_APPROX s_log10_approx_tests[] =
{
//  {    x,                     {    y_rounded,               y_difference           } }
    {                 0x0.0p+0, {                -INFINITY,                 0x0.0p+0 }, 1 }, // log(0.0) == -inf
    {    0x1.0000000000000p-54, {    -0x1.041704c068eefp+4,   -0x1.d17237c752536p-52 }, 1 }, // log10(5.551115123125783e-17) == -16.255619765854984542
    {    0x1.0000000000000p-53, {    -0x1.fe8bffd88220ep+3,   -0x1.0a56ddbe42798p-51 }, 1 }, // log10(1.1102230246251565e-16) == -15.954589770191003346
    {    0x1.8000000000000p-53, {    -0x1.f8e975b5a9f2ep+3,   -0x1.47a0bb7b34258p-51 }, 1 }, // log10(1.6653345369377348e-16) == -15.778498511135322104
    {    0x1.0000000000000p-52, {    -0x1.f4e9f6303263ep+3,   -0x1.2bf49f98dbc95p-51 }, 1 }, // log10(2.220446049250313e-16) == -15.653559774527022151
    {    0x1.0000000000000p-52, {    -0x1.f4e9f6303263ep+3,   -0x1.2bf49f98dbc95p-51 }, 1 }, // log10(2.220446049250313e-16) == -15.653559774527022151
    {     0x1.fae147ae147b4p-3, {    -0x1.367d4f9ae1922p-1,   -0x1.af855331d7859p-56 }, 1 }, // log10(0.24750000000000016) == -0.60642479673041218676
    {     0x1.fae147ae147b0p-2, {    -0x1.38b96a2bcb849p-2,    0x1.31516a2d33a18p-60 }, 1 }, // log10(0.4950000000000001) == -0.30539480106643118636
    {     0x1.7c28f5c28f5c3p-1, {    -0x1.08d04ba18b898p-3,   -0x1.102c5263181e2p-58 }, 1 }, // log10(0.7425) == -0.12930354201075000922
    {     0x1.fae147ae147aep-1, {    -0x1.1e0d4874f92f2p-8,   -0x1.5e74871b18f79p-62 }, 1 }, // log10(0.99) == -0.004364805402450088556
    {     0x1.0000000000000p+0, {                 0x0.0p+0,                 0x0.0p+0 }, 1 }, // log10(1.0) == 0.0
    {     0x1.9cccccccccccdp+1, {     0x1.045e01e00d2ebp-1,   -0x1.796443a47d512p-55 }, 1 }, // log10(3.225) == 0.50852971897128658523
    {     0x1.5cccccccccccdp+2, {     0x1.7908f65c557cfp-1,    0x1.0d7df3076a7fdp-56 }, 1 }, // log10(5.45) == 0.73639650227664245414
    {     0x1.eb33333333334p+2, {     0x1.c528fe74a287dp-1,    0x1.a410b65224725p-58 }, 1 }, // log10(7.675000000000001) == 0.88507838414922413453
    {     0x1.3cccccccccccdp+3, {     0x1.fdc3e56f160dap-1,    0x1.20f228496591ap-55 }, 1 }, // log10(9.9) == 0.99563519459754993093
    {     0x1.4000000000000p+3, {     0x1.0000000000000p+0,                 0x0.0p+0 }, 1 }, // log10(10.0) == 1.0
    {     0x1.03ccccccccccdp+5, {     0x1.82f4e2bf66bdfp+0,    0x1.6d8b05439f637p-54 }, 1 }, // log10(32.475) == 1.5115491597450655086
    {     0x1.b79999999999ap+5, {     0x1.bd6e85e1adb4bp+0,   -0x1.3cdef74d0afd1p-54 }, 1 }, // log10(54.95) == 1.7399676967595093947
    {     0x1.35b3333333334p+6, {     0x1.e38db82048570p+0,   -0x1.1cfbf3bb4f3afp-55 }, 1 }, // log10(77.42500000000001) == 1.8888812140288244425
    {     0x1.8f9999999999ap+6, {     0x1.ffe3861a240fep+0,   -0x1.ce61059d0cd29p-57 }, 1 }, // log10(99.9) == 1.9995654882259823334
    {     0x1.9000000000000p+6, {     0x1.0000000000000p+1,                 0x0.0p+0 }, 1 }, // log10(100.0) == 2.0
    {     0x1.44f999999999ap+8, {     0x1.41844c9b1087ep+1,    0x1.e9820ed2bd55ap-53 }, 1 }, // log10(324.975) == 2.5118499524260748848
    {     0x1.12f999999999ap+9, {     0x1.5ec2e92bf1419p+1,    0x1.8bacaab4bb7c9p-53 }, 1 }, // log10(549.95) == 2.7403232063829931537
    {     0x1.8376666666666p+9, {     0x1.71d342cc6400ep+1,    0x1.f6d7fe2385e25p-53 }, 1 }, // log10(774.925) == 2.889259671974293338
    {     0x1.f3f3333333333p+9, {     0x1.7ffe93ab4e811p+1,    0x1.8268646d6e9b2p-53 }, 1 }, // log10(999.9) == 2.9999565683801924797
    {     0x1.f400000000000p+9, {     0x1.8000000000000p+1,                 0x0.0p+0 }, 1 }, // log10(1000.0) == 3.0
    {    0x1.963f333333333p+11, {     0x1.c18548d533a15p+1,   -0x1.e3ec706c8ae0dp-56 }, 1 }, // log10(3249.975) == 3.5118800202392414804
    {    0x1.57bf333333333p+12, {     0x1.dec41342b1052p+1,    0x1.51b4aceb3c9a6p-55 }, 1 }, // log10(5499.95) == 3.7403587413446440035
    {    0x1.e45ecccccccccp+12, {     0x1.f1d4801ead08dp+1,   -0x1.d95ccf4d98006p-56 }, 1 }, // log10(7749.924999999999) == 3.889297499636148877
    {    0x1.387f333333333p+13, {     0x1.ffffdb918ce60p+1,    0x1.582eb6532b2f4p-55 }, 1 }, // log10(9999.9) == 3.9999956570334660828
    {    0x1.3880000000000p+13, {     0x1.0000000000000p+2,                 0x0.0p+0 }, 1 }, // log10(10000.0) == 4.0
    {    0x1.fbcfe66666666p+14, {     0x1.20c2b106fc196p+2,   -0x1.d07276054afefp-53 }, 1 }, // log10(32499.975) == 4.5118830269060674827
    {    0x1.adafe66666666p+15, {     0x1.2f621888b6248p+2,   -0x1.ee91317318d08p-52 }, 1 }, // log10(54999.95) == 4.7403622946808989953
    {    0x1.2ebbeccccccccp+16, {     0x1.38ea4fecdb8eep+2,    0x1.aeff29ed9ce4bp-58 }, 1 }, // log10(77499.92499999999) == 4.8893012822211243732
    {    0x1.869fe66666666p+16, {     0x1.3ffffe2dadfb7p+2,   -0x1.dac48afd82008p-53 }, 1 }, // log10(99999.9) == 4.9999995657053009241
    {    0x1.86a0000000000p+16, {     0x1.4000000000000p+2,                 0x0.0p+0 }, 1 }, // log10(100000.0) == 5.0
    {    0x1.3d61fe6666666p+18, {     0x1.60c2b249d269bp+2,   -0x1.2bfb443be7da5p-52 }, 1 }, // log10(324999.975) == 5.5118833275716052241
    {    0x1.0c8dfe6666666p+19, {     0x1.6f621a063f10fp+2,    0x1.021f0c76aa49fp-55 }, 1 }, // log10(549999.95) == 5.7403626500129254775
    {    0x1.7a6afd999999ap+19, {     0x1.78ea518301f6dp+2,    0x1.3a8f2e084550ep-53 }, 1 }, // log10(774999.925) == 5.889301660477810033
    {    0x1.e847fcccccccdp+19, {     0x1.7fffffd15e342p+2,   -0x1.aedb5baa85663p-54 }, 1 }, // log10(999999.9) == 5.9999999565705496483
};

void Test_log10_approx(void)
{
    for (int i = 0; i < _countof(s_log10_approx_tests); i++)
    {
        double x = s_log10_approx_tests[i].x;
        double expected = s_log10_approx_tests[i].expected.rounded;
        double z = log10(x);
        int64_t error = abs(ulp_error_precise(&s_log10_approx_tests[i].expected, z));
        ok(error <= s_log10_approx_tests[i].max_error,
            "log10(%.17e) = %.17e, expected %.17e, error %I64d ULPs, max %u ULPs\n",
            x, z, expected, error, s_log10_approx_tests[i].max_error);
    }
}

#if defined(HAS_LOG10F)

// These are expected to match exactly
static TESTENTRY_DBL s_log10f_exact_tests[] =
{
    { 0x00000000 /*  0.000000 */, 0xff800000 /* -1.#IND00 */ },
    { 0x80000000 /* -0.000000 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7f800000 /*  1.#INF00 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7f800001 /*  1.#SNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7fBFffff /*  1.#SNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7fC00000 /*  1.#QNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7fC80001 /*  1.#QNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0x7fFfffff /*  1.#QNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0xff800000 /* -1.#INF00 */, 0xff800000 /* -1.#IND00 */ },
    { 0xff800001 /* -1.#SNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0xffBfffff /* -1.#SNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0xffC00000 /* -1.#IND00 */, 0xff800000 /* -1.#IND00 */ },
    { 0xfff80001 /* -1.#QNAN0 */, 0xff800000 /* -1.#IND00 */ },
    { 0xffffffff /* -1.#QNAN0 */, 0xff800000 /* -1.#IND00 */ },
};

void Test_log10f_exact(void)
{
    for (int i = 0; i < _countof(s_log10f_exact_tests); i++)
    {
        float x = u64_to_dbl(s_log10f_exact_tests[i].x);
        float z = log10f(x);
        ok_eq_flt_exact("log10f", s_log10f_exact_tests[i].x, z, s_log10f_exact_tests[i].result);
    }
}

#endif // defined(HAS_LOG10F)

#if defined(HAS_LOG10F) || defined(HAS_LIBM_SSE2)

static TESTENTRY_DBL_APPROX s_log10f_approx_tests[] =
{
//  {    x,                     {    y_rounded,               y_difference           } }
    {                 0x0.0p+0, {                -INFINITY,                 0x0.0p+0 }, 1 }, // logf(0.0) == -inf
    {    0x1.0000000000000p-54, {    -0x1.041704c068eefp+4,   -0x1.d17237c752536p-52 }, 1 }, // log10f(5.551115123125783e-17) == -16.255619765854984542
    {    0x1.0000000000000p-53, {    -0x1.fe8bffd88220ep+3,   -0x1.0a56ddbe42798p-51 }, 1 }, // log10f(1.1102230246251565e-16) == -15.954589770191003346
    {    0x1.8000000000000p-53, {    -0x1.f8e975b5a9f2ep+3,   -0x1.47a0bb7b34258p-51 }, 1 }, // log10f(1.6653345369377348e-16) == -15.778498511135322104
    {    0x1.0000000000000p-52, {    -0x1.f4e9f6303263ep+3,   -0x1.2bf49f98dbc95p-51 }, 1 }, // log10f(2.220446049250313e-16) == -15.653559774527022151
    {    0x1.0000000000000p-52, {    -0x1.f4e9f6303263ep+3,   -0x1.2bf49f98dbc95p-51 }, 1 }, // log10f(2.220446049250313e-16) == -15.653559774527022151
    {     0x1.fae1480000000p-3, {    -0x1.367d4f76f1c21p-1,    0x1.39407b2675a98p-55 }, 1 }, // log10f(0.2475000023841858) == -0.60642479254682165536
    {     0x1.fae1480000000p-2, {    -0x1.38b969e3ebe42p-2,   -0x1.c13745063ea74p-56 }, 1 }, // log10f(0.4950000047683716) == -0.30539479688284046014
    {     0x1.7c28f60000000p-1, {    -0x1.08d04b11cc489p-3,    0x1.2b1a06b717b31p-57 }, 1 }, // log10f(0.7425000071525574) == -0.12930353782715921806
    {     0x1.fae1480000000p-1, {    -0x1.1e0d367d110dfp-8,   -0x1.3be0165a28612p-62 }, 1 }, // log10f(0.9900000095367432) == -0.0043648012188592649305
    {     0x1.0000000000000p+0, {                 0x0.0p+0,                 0x0.0p+0 }, 1 }, // log10f(1.0) == 0.0
    {     0x1.9ccccc0000000p+1, {     0x1.045e0171bbe4dp-1,   -0x1.0573bd9148ad2p-55 }, 1 }, // log10f(3.2249999046325684) == 0.50852970612863543306
    {     0x1.5ccccc0000000p+2, {     0x1.7908f5d9c64fcp-1,    0x1.ab1f7f7cb932bp-55 }, 1 }, // log10f(5.449999809265137) == 0.73639648707754150804
    {     0x1.eb33340000000p+2, {     0x1.c528fed158404p-1,    0x1.e8799c9e91e21p-55 }, 1 }, // log10f(7.675000190734863) == 0.8850783949420706872
    {     0x1.3ccccc0000000p+3, {     0x1.fdc3e4df56cc6p-1,    0x1.d6269bf6a3dbfp-58 }, 1 }, // log10f(9.899999618530273) == 0.99563517786318623341
    {     0x1.4000000000000p+3, {     0x1.0000000000000p+0,                 0x0.0p+0 }, 1 }, // log10f(10.0) == 1.0
    {     0x1.03cccc0000000p+5, {     0x1.82f4e267c23eap+0,    0x1.254e66dd840e4p-54 }, 1 }, // log10f(32.474998474121094) == 1.5115491393391900591
    {     0x1.b7999a0000000p+5, {     0x1.bd6e85fb93994p+0,   -0x1.40002f211d53ep-55 }, 1 }, // log10f(54.95000076293945) == 1.7399677027893618239
    {     0x1.35b3340000000p+6, {     0x1.e38db869cdc91p+0,    0x1.808cc14d0d232p-54 }, 1 }, // log10f(77.42500305175781) == 1.8888812311468295703
    {     0x1.8f999a0000000p+6, {     0x1.ffe38636a19bap+0,   -0x1.f1ebee0c7da99p-54 }, 1 }, // log10f(99.9000015258789) == 1.9995654948594235897
    {     0x1.9000000000000p+6, {     0x1.0000000000000p+1,                 0x0.0p+0 }, 1 }, // log10f(100.0) == 2.0
    {     0x1.44f99a0000000p+8, {     0x1.41844cac94b93p+1,   -0x1.6a2bdd2b6f0acp-54 }, 1 }, // log10f(324.9750061035156) == 2.5118499605827734658
    {     0x1.12f99a0000000p+9, {     0x1.5ec2e940a4d4ap+1,   -0x1.0adc66eb93085p-53 }, 1 }, // log10f(549.9500122070312) == 2.7403232160228626577
    {     0x1.8376660000000p+9, {     0x1.71d342bdb2fe4p+1,    0x1.a8a72c4b6a864p-55 }, 1 }, // log10f(774.9249877929688) == 2.8892596651330553675
    {     0x1.f3f3340000000p+9, {     0x1.7ffe93c21416dp+1,   -0x1.d45fc1cda5430p-53 }, 1 }, // log10f(999.9000244140625) == 2.9999565789841453801
    {     0x1.f400000000000p+9, {     0x1.8000000000000p+1,                 0x0.0p+0 }, 1 }, // log10f(1000.0) == 3.0
    {    0x1.963f340000000p+11, {     0x1.c18548f139d76p+1,    0x1.f52ca2d5fbc37p-55 }, 1 }, // log10f(3249.97509765625) == 3.5118800332890556792
    {    0x1.57bf340000000p+12, {     0x1.dec41363cfa53p+1,   -0x1.ff13fb71e031bp-53 }, 1 }, // log10f(5499.9501953125) == 3.740358756767173221
    {    0x1.e45ecc0000000p+12, {     0x1.f1d480072bf12p+1,    0x1.9d279c034298ep-54 }, 1 }, // log10f(7749.9248046875) == 3.8892974886911214413
    {    0x1.387f340000000p+13, {     0x1.ffffdbb5fb650p+1,    0x1.5ad4069a247b3p-59 }, 1 }, // log10f(9999.900390625) == 3.9999956739982636146
    {    0x1.3880000000000p+13, {     0x1.0000000000000p+2,                 0x0.0p+0 }, 1 }, // log10f(10000.0) == 4.0
    {    0x1.fbcfe60000000p+14, {     0x1.20c2b10161445p+2,   -0x1.e2e44fb30b24fp-52 }, 1 }, // log10f(32499.974609375) == 4.5118830216861778556
    {    0x1.adafe60000000p+15, {     0x1.2f621882166e7p+2,   -0x1.ea0ac70988daep-52 }, 1 }, // log10f(54999.94921875) == 4.7403622885119376575
    {    0x1.2ebbec0000000p+16, {     0x1.38ea4fda0ded1p+2,   -0x1.f64fd88f2e155p-58 }, 1 }, // log10f(77499.921875) == 4.889301264709232868
    {    0x1.869fe60000000p+16, {     0x1.3ffffe2664b32p+2,   -0x1.d0caeeccbf02cp-53 }, 1 }, // log10f(99999.8984375) == 4.9999995589194428308
    {    0x1.86a0000000000p+16, {     0x1.4000000000000p+2,                 0x0.0p+0 }, 1 }, // log10f(100000.0) == 5.0
    {    0x1.3d61fe0000000p+18, {     0x1.60c2b240daaecp+2,   -0x1.17bafb99593c2p-57 }, 1 }, // log10f(324999.96875) == 5.5118833192197875727
    {    0x1.0c8dfe0000000p+19, {     0x1.6f6219fba5ee3p+2,    0x1.227f4a8b4322fp-52 }, 1 }, // log10f(549999.9375) == 5.7403626401425953706
    {    0x1.7a6afe0000000p+19, {     0x1.78ea518a876a5p+2,   -0x1.0f08360f8c90fp-53 }, 1 }, // log10f(774999.9375) == 5.8893016674825603364
    {    0x1.e847fc0000000p+19, {     0x1.7fffffc5b5c12p+2,   -0x1.a3921eb92e9aep-52 }, 1 }, // log10f(999999.875) == 5.9999999457131863692
};

#endif // defined(HAS_LOG10F) || defined(HAS_LIBM_SSE2)

#if defined(HAS_LOG10F)

void Test_log10f_approx(void)
{
    for (int i = 0; i < _countof(s_log10f_approx_tests); i++)
    {
        float x = s_log10f_approx_tests[i].x;
        float expected = s_log10f_approx_tests[i].expected.rounded;
        float z = log10f(x);
        int64_t error = abs(ulp_error_flt(expected, z));
        ok(error <= s_log10_approx_tests[i].max_error,
            "log10(%.6e) = %.7e, expected %.7e, error %I64d ULPs, max %u ULPs\n",
            x, z, expected, error, s_log10_approx_tests[i].max_error);
    }
}

#endif // defined(HAS_LOG10F)

#if defined(HAS_LIBM_SSE2)

__ATTRIBUTE_SSE2__ __m128d __libm_sse2_log10(__m128d Xmm0);

__ATTRIBUTE_SSE2__
void Test_libm_sse2_log10(void)
{
    int i;
    for (i = 0; i < _countof(s_log10_approx_tests); i++)
    {
        double x = s_log10_approx_tests[i].x;
        double expected = s_log10_approx_tests[i].expected.rounded;
        __m128d xmm0 = _mm_set_sd(x);
        __m128d xmm1 = __libm_sse2_log10(xmm0);
        double z = _mm_cvtsd_f64(xmm1);
        int64_t error = ulp_error_precise(&s_log10_approx_tests[i].expected, z);
        ok(error <= s_log10_approx_tests[i].max_error,
            "__libm_sse2_log10(%.17e) = %.17e, expected %.17e, error %I64d ULPs, max %u ULPs\n",
            x, z, expected, error, s_log10_approx_tests[i].max_error);
    }
}

__ATTRIBUTE_SSE2__ __m128 __libm_sse2_log10f(__m128 Xmm0);

__ATTRIBUTE_SSE2__
void Test_libm_sse2_log10f(void)
{
    int i;
    for (i = 0; i < _countof(s_log10f_approx_tests); i++)
    {
        float x = s_log10f_approx_tests[i].x;
        float expected = s_log10f_approx_tests[i].expected.rounded;
        __m128 xmm0 = _mm_set_ps1(x);
        __m128 xmm1 = __libm_sse2_log10f(xmm0);
        float z = _mm_cvtss_f32(xmm1);
        int64_t error = ulp_error_flt(expected, z);
        ok(error <= s_log10f_approx_tests[i].max_error,
            "__libm_sse2_log10f(%.6e) = %.7e, expected %.7e, error %I64d ULPs, max %u ULPs\n",
            x, z, expected, error, s_log10f_approx_tests[i].max_error);
    }
}

#endif // defined(HAS_LIBM_SSE2)

START_TEST(log10)
{
    Test_log10_exact();
    Test_log10_approx();
#if defined(HAS_LOG10F)
    Test_log10f_exact();
    Test_log10f_approx();
#endif
#if defined(HAS_LIBM_SSE2)
    Test_libm_sse2_log10();
    Test_libm_sse2_log10f();
#endif
}
