      real x,y
     +    ,sig1g_u,sigrad_u
     +    ,tbor_u,tine_u,tnuc_u,tpro_u
     +    ,sig1g_p,sigrad_p
     +    ,tbor_p,tine_p,tnuc_p,tpro_p
     +    ,vac_u,vertex_u,small_u,redfac_u
     +    ,sig1g_udis,sigrad_udis
     +    ,tbor_udis,tine_udis,tnuc_udis,tpro_udis
     +    ,sig1g_pdis,sigrad_pdis
     +    ,tbor_pdis,tine_pdis,tnuc_pdis,tpro_pdis
     +    ,vac_udis,vertex_udis,small_udis,redfac_udis
     +    ,sig1g_upho,sigrad_upho
     +    ,tbor_upho,tine_upho,tnuc_upho,tpro_upho
     +    ,sig1g_ppho,sigrad_ppho
     +    ,tbor_ppho,tine_ppho,tnuc_ppho,tpro_ppho
     +    ,vac_upho,vertex_upho,small_upho,redfac_upho
      common/xytab/x(nt),y(nt)
     +     ,sig1g_u(nt,nt),sigrad_u(nt,nt),tbor_u(nt,nt)
     +     ,tine_u(nt,nt),tnuc_u(nt,nt),tpro_u(nt,nt)
     +     ,sig1g_p(nt,nt),sigrad_p(nt,nt),tbor_p(nt,nt)
     +     ,tine_p(nt,nt),tnuc_p(nt,nt),tpro_p(nt,nt)
     +     ,vac_u(nt,nt),vertex_u(nt,nt),small_u(nt,nt)
     +     ,redfac_u(nt,nt)
     +     ,sig1g_udis(ntdis,ntdis),sigrad_udis(ntdis,ntdis)
     +     ,tbor_udis(ntdis,ntdis),tine_udis(ntdis,ntdis)
     +     ,tnuc_udis(ntdis,ntdis),tpro_udis(ntdis,ntdis)
     +     ,sig1g_pdis(ntdis,ntdis),sigrad_pdis(ntdis,ntdis)
     +     ,tbor_pdis(ntdis,ntdis),tine_pdis(ntdis,ntdis)
     +     ,tnuc_pdis(ntdis,ntdis),tpro_pdis(ntdis,ntdis)
     +     ,vac_udis(ntdis,ntdis),vertex_udis(ntdis,ntdis)
     +     ,small_udis(ntdis,ntdis),redfac_udis(ntdis,ntdis)
     +     ,sig1g_upho(ntpho,ntpho),sigrad_upho(ntpho,ntpho)
     +     ,tbor_upho(ntpho,ntpho),tine_upho(ntpho,ntpho)
     +     ,tnuc_upho(ntpho,ntpho),tpro_upho(ntpho,ntpho)
     +     ,sig1g_ppho(ntpho,ntpho),sigrad_ppho(ntpho,ntpho)
     +     ,tbor_ppho(ntpho,ntpho),tine_ppho(ntpho,ntpho)
     +     ,tnuc_ppho(ntpho,ntpho),tpro_ppho(ntpho,ntpho)
     +     ,vac_upho(ntpho,ntpho),vertex_upho(ntpho,ntpho)
     +     ,small_upho(ntpho,ntpho),redfac_upho(ntpho,ntpho)
