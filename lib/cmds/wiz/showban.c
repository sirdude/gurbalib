void main( string str ) {
   string *sites;
   int i;
   sites = SITEBAN_D->query_banned_sites();
   for( i=0; i<sizeof(sites); i++) {
      write(sites[i]+"\n");
   }
}
