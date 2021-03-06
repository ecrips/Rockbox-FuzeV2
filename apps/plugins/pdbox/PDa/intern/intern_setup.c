#include <stdio.h>

void d_intern_setup() {
    fprintf(stderr,"setup\n");
    biquad_tilde_setup();
    bp_tilde_setup();
    clip_tilde_setup();
    cos_tilde_setup();
    dbtopow_tilde_setup();
    dbtorms_tilde_setup();
    delread_tilde_setup();
    delwrite_tilde_setup();
    env_tilde_setup();
    ftom_tilde_setup();
    hip_tilde_setup();
    line_tilde_setup();
    lop_tilde_setup();
    mtof_tilde_setup();
    noise_tilde_setup();
    osc_tilde_setup();
    phasor_tilde_setup();
    powtodb_tilde_setup();
    print_tilde_setup();
    rmstodb_tilde_setup();
    rsqrt_tilde_setup();
    samphold_tilde_setup();
    sfread_tilde_setup();
    sfwrite_tilde_setup();
    sig_tilde_setup();
    snapshot_tilde_setup();
    sqrt_tilde_setup();
    tabosc4_tilde_setup();
    tabplay_tilde_setup();
    tabread4_tilde_setup();
    tabread_tilde_setup();
    tabread_setup();
    tabreceive_tilde_setup();
    tabsend_tilde_setup();
    tabwrite_tilde_setup();
    tabwrite_setup();
    threshold_tilde_setup();
    vcf_tilde_setup();
    vd_tilde_setup();
    vline_tilde_setup();
    vsnapshot_tilde_setup();
    wrap_tilde_setup(); 
}

