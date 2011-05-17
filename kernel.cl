#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void popcorn( __global int *iDevice, __global int *fDevice, __global double *oDevice, int ncharges )
{
  //this worker's work group id
  int work_group_id = get_group_id(0);

  //this worker's id
  int global_work_item_id = get_global_id(0);

  //this worker's local id in its work group
  int local_work_item_id = get_local_id(0);

  //calculate this work group's initial position 
  int i_id = iDevice[work_group_id];
  int   zi = ( i_id ) / ( ASIZE );
  int   yi = ( i_id / XSIZE - zi * YSIZE );
  int   xi = ( i_id % XSIZE );

  //calculate this work group's final position
  int f_id = fDevice[work_group_id];
  int   zf = ( f_id / ASIZE );
  int   yf = ( f_id / XSIZE - zf * YSIZE );
  int   xf = ( f_id % XSIZE );

  //to store the potential calculated by this worker
  double v = 0;

  //to store the distance calcualted by this worker
  double r = 0;

  //to store an interacting charge's position
  int z = 0;
  int y = 0;
  int x = 0;

  __local int    sharedI[1024];
  __local double sharedF[1024];

  //for ( int load_number= 0; load_number < get_num_groups(0) / get_local_size(0) + 1; load_number++ )
  for ( int load_number= 0; load_number < ncharges / get_local_size(0) + 1; load_number++ )
  {
   //this worker is responsible for loading from global using this load id
   int load_id = get_local_size(0) * load_number + local_work_item_id;

   //make sure the load id calculated is actually in range
   //if ( load_id < get_num_groups(0) )
   if ( load_id < ncharges )
   {
    sharedI[local_work_item_id] = iDevice[load_id];
   }
   else
   {
    sharedI[local_work_item_id] = -1; // -1 means a non-interacting charge
   }
   barrier(CLK_LOCAL_MEM_FENCE);

   //perform a calculation only if an interacting charge was loaded for this worker
   if ( sharedI[local_work_item_id] >= 0 )
   {
    //do not interact with yourself now or in the future
    if ( load_id != work_group_id )
    {
     //the interacting charge's position now
     z =   sharedI[local_work_item_id] / ASIZE;
     y = ( sharedI[local_work_item_id] / XSIZE - z * YSIZE );
     x = ( sharedI[local_work_item_id] % XSIZE );
     //future
     r  = (x-xf)*(x-xf)+(y-yf)*(y-yf)+(z-zf)*(z-zf);
     if ( r < CUTOFF2 && r > 0 ) { v += rsqrt(r); }
     //now
     r  = (x-xi)*(x-xi)+(y-yi)*(y-yi)+(z-zi)*(z-zi);
     if ( r < CUTOFF2 && r > 0 ) { v -= rsqrt(r); }
    }
   }
   barrier(CLK_LOCAL_MEM_FENCE);

  }

  barrier(CLK_LOCAL_MEM_FENCE);
  sharedF[local_work_item_id] = v;
  barrier(CLK_LOCAL_MEM_FENCE);

  if ( local_work_item_id == 0 )
  {
   v = 0;
   for ( int i = 0; i < get_local_size(0); i++ ) v += sharedF[i];
   oDevice[work_group_id] = PREFACTOR * v;
  }

}
