
import sbms

Import('*')

subdirs = ['genr8', 'genr8_2_hddm', 'HDGeant', 'mcsmear', 'bggen', 'gen_2k', 'gen_2pi', 'gen_2pi_amp', 'gen_2pi_primakoff','gen_3pi', 'gen_pi0', 'gen_omega_3pi', 'gen_omega_radiative' , 'nullgen', 'BGRate_calc']

sbms.OptionallyBuild(env, ['genphoton', 'genpi', 'gen_2mu', 'genEtaRegge', 'gen_ee', 'gen_ee_hb', 'stdhep_translators'])

SConscript(dirs=subdirs, exports='env osname', duplicate=0)
