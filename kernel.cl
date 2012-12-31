#pragma OPENCL EXTENSION cl_khr_fp64 : enable

// The kernel calculates the coulomb potential at every point on a 3D rectangular grid of size ( Wx * Wy * Wz ).
// The calculation is performed using a (larger) computational grid of size ( Sx * Sy * Sz ) * ( Wx * Wy * Wz ) = ( Gx * Gy * Gz ).
//
// Wx ( along x-direction ) : number of WORK GROUPS                              : get_num_groups(0)
// Sx ( along x-direction ) : number of WORK ITEMS locally ( per WORK GROUP )    : get_local_size(0)
// Gx ( along x-direction ) : number of WORK ITEMS globally                      : get_global_size(0)
//
// wx ( along x-direction ) : WORK GROUP id                                      : get_group_id(0)
// sx ( along x-direction ) : WORK ITEM local id                                 : get_local_id(0)
// gx ( along x-direction ) : WORK ITEM global id                                : get_global_id(0)
//
// Extensive use is made of the following relations for a 3D to 1D index mapping:
//     hash    : a, b, c
//     3D index: x, y, z
//     1D index: s
//
// The hash numbers can be any positive set of non-zero integers - for example they could be the grid dimensions, the work group
// dimensions, etc.  Note that '/' means integer division and '%' is the modulo operation in the following formula.  These formula
// allow us to map between information on a 3D grid and information in a 1D vector.  This process could be avoided if we chose to
// use a 1D computational grid.
//
// To map from (x,y,z) to s:
//     s = x + y * a + z * a * b
//
// To map from s to (x,y,z):
//     z = s / ( a * b )
//     y = s / ( a ) - ( z * b )
//     x = s % ( a )
//
// In the algorithm, the following mappins are used:
//
//     hash     : ( Sx, Sy, Sz )   ... ( get_local_size(0), get_local_size(1), get_local_size(2) )
//     3D index : ( sx, sy, sz )   ... ( get_local_id(0), get_local_id(1), get_local_id(2) )
//     1D index :    j
//
//     hash     : ( Wx, Wy, Wz )   ... ( get_num_groups(0), get_num_groups(1), get_num_groups(2) )
//     3D index : ( wx, wy, wz )   ... ( get_group_id(0), get_group_id(1), get_group_id(2) )
//     1D index :    k
//
//     hash     : ( Wx, Wy, Wz )   ... ( get_num_groups(0), get_num_groups(1), get_num_groups(2) )
//     3D index : (  x,  y,  z )   ... ( get_group_id(0), get_group_id(1), get_group_id(2) )
//     1D index :    s
//     Allows a single work group to access the a single index in the global memory.
//
// Each WORK GROUP calculates the coulomb potential ( as the sum of q / r ) for a single point in 3D space due to a set of charges
// located at 'sites s' with 'charges q'.  The 'sites s' are positive integer numbers.  The 'charges q' are integer numbers.
//
// Work items in a group work together to copy 'sites s' and 'charges q' from global memory to local memory in pieces.  Each work
// item then calculates the distance between the its work group id (wx,wy,wz) and the single 'site s' that it copied.  Each work
// item in the work group sums up q / r terms ( only the ones its responsible for ).  Finally, the first work item in each group
// sums up all the other work items q / r sums for the given work group and writes the answer to the global memory 'o'.

// coulomb1 calcules the coulomb interaction EVERYWHERE
__kernel void coulomb1( __global double *o, __global int *s, __global int *q, int n, int c2, double prefactor )
{
    // map 3D local work item indecies to 1D index j
    int j = get_local_id(0) +
            get_local_id(1) * get_local_size(0) +
            get_local_id(2) * get_local_size(0) * get_local_size(1);

    // map 3D work group indecies to 1D index k
    int k = get_group_id(0) +
            get_group_id(1) * get_num_groups(0) +
            get_group_id(2) * get_num_groups(0) * get_num_groups(1);

    // let 'this work item' know about the local memory for the work group it belongs to
    // ... to be accessed using the index 'j'
    __local int    slocal[64];
    __local int    qlocal[64];
    __local double vlocal[64];

    // have 'this work item' set its own initial potential to zero
    vlocal[j] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    // loop over global memory in chunks of size local_volume = ( Sx * Sy * Sz )
    // There are n charges, so you need to do at least n / local_volume loads, maybe 1 more if the numbers don't divide nice.
    int local_volume = get_local_size(0) * get_local_size(1) * get_local_size(2);
    int num_loads = n / ( local_volume ) + 1;

    for ( int load_number = 0; load_number < num_loads; load_number++ )
    {
        // 'this work item' is responsible for loading information for a single charge from the arrays s and q
        int load_id = local_volume * load_number + j;
        if ( load_id < n ) //number of charges
        {
            slocal[j] = s[load_id];
            qlocal[j] = q[load_id];
        }
        else
        {
            // don't load from s and q if our load_id was greater than the number of charges in s and q - it can happend when
            // the numbers don't divide nicely.
            slocal[j] = -1;
            qlocal[j] = -1;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // 'this work item now calculates a q / r for the local id it loaded ... only if it loaded one ( s >= 0 )
        int s = slocal[j];
        if ( s >= 0 )
        {
            // calculate the x,y,z for the charge 'this work item' loaded from the 1D index s
            int   z = ( s ) / ( get_num_groups(0) * get_num_groups(1) );
            int   y = ( s ) / ( get_num_groups(0) ) - ( z * get_num_groups(1) );
            int   x = ( s ) % ( get_num_groups(0) );
            // calcualte the distance between x,y,z and 'this work group' - remember each point in the 3D space we are calculating
            // the coulomb potential in got assigned to a work group; The assignment was done in such a way so that the work group
            // ids corresponded to the position of the point the work group is assigned to.
            double r = ( get_group_id(0) - x ) * ( get_group_id(0) - x ) +
                       ( get_group_id(1) - y ) * ( get_group_id(1) - y ) +
                       ( get_group_id(2) - z ) * ( get_group_id(2) - z );
            // Check for cutoff and make sure r != 0 ( which happens when a charge is present at the work groups position )
            if ( r > 0 && r < c2 )
            {
                vlocal[j] = vlocal[j] + qlocal[j] * rsqrt(r);
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // If 'this work item' is work item 0 in the work group, tally up the results from the other work groups and write them to the
    // output vector o.
    if ( j == 0 )
    {
        double v = 0;
        for ( int l = 0; l < local_volume; l++ )
        {
            v = v + vlocal[l];
        }
        o[k] = prefactor * v;
    }
}

//gauss1 calcualtes the coulomb interaction with erf EVERYWHERE
__kernel void gauss1( __global double *o, __global int *s, __global int *q, int n, int c2, double prefactor, double erffactor )
{
    // map 3D local work item indecies to 1D index j
    int j = get_local_id(0) +
            get_local_id(1) * get_local_size(0) +
            get_local_id(2) * get_local_size(0) * get_local_size(1);

    // map 3D work group indecies to 1D index k
    int k = get_group_id(0) +
            get_group_id(1) * get_num_groups(0) +
            get_group_id(2) * get_num_groups(0) * get_num_groups(1);

    // let 'this work item' know about the local memory for the work group it belongs to
    // ... to be accessed using the index 'j'
    __local int    slocal[64];
    __local int    qlocal[64];
    __local double vlocal[64];

    // have 'this work item' set its own initial potential to zero
    vlocal[j] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    // loop over global memory in chunks of size local_volume = ( Sx * Sy * Sz )
    // There are n charges, so you need to do at least n / local_volume loads, maybe 1 more if the numbers don't divide nice.
    int local_volume = get_local_size(0) * get_local_size(1) * get_local_size(2);
    int num_loads = n / ( local_volume ) + 1;

    for ( int load_number = 0; load_number < num_loads; load_number++ )
    {
        // 'this work item' is responsible for loading information for a single charge from the arrays s and q
        int load_id = local_volume * load_number + j;
        if ( load_id < n ) //number of charges
        {
            slocal[j] = s[load_id];
            qlocal[j] = q[load_id];
        }
        else
        {
            // don't load from s and q if our load_id was greater than the number of charges in s and q - it can happend when
            // the numbers don't divide nicely.
            slocal[j] = -1;
            qlocal[j] = -1;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // 'this work item now calculates a q / r for the local id it loaded ... only if it loaded one ( s >= 0 )
        int s = slocal[j];
        if ( s >= 0 )
        {
            // calculate the x,y,z for the charge 'this work item' loaded from the 1D index s
            int   z = ( s ) / ( get_num_groups(0) * get_num_groups(1) );
            int   y = ( s ) / ( get_num_groups(0) ) - ( z * get_num_groups(1) );
            int   x = ( s ) % ( get_num_groups(0) );
            // calcualte the distance between x,y,z and 'this work group' - remember each point in the 3D space we are calculating
            // the coulomb potential in got assigned to a work group; The assignment was done in such a way so that the work group
            // ids corresponded to the position of the point the work group is assigned to.
            double r = ( get_group_id(0) - x ) * ( get_group_id(0) - x ) +
                       ( get_group_id(1) - y ) * ( get_group_id(1) - y ) +
                       ( get_group_id(2) - z ) * ( get_group_id(2) - z );
            // Check for cutoff and make sure r != 0 ( which happens when a charge is present at the work groups position )
            if ( r > 0 && r < c2 )
            {
                r = sqrt(r);
                vlocal[j] = vlocal[j] + qlocal[j] * erf(erffactor * r) / r;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // If 'this work item' is work item 0 in the work group, tally up the results from the other work groups and write them to the
    // output vector o.
    if ( j == 0 )
    {
        double v = 0;
        for ( int l = 0; l < local_volume; l++ )
        {
            v = v + vlocal[l];
        }
        o[k] = prefactor * v;
    }
}

__kernel void coulomb2( __global double *o, __global int *s, __global int *q, int n, int c2, __global int *w, int xsize, int ysize, double prefactor )
{
    // each worker of work group loads the same charge and site, using the "work group id"
    int qi = q[ get_group_id(0) ];
    int si = w[ get_group_id(0) ];

    // extract position from site using the grid dimensions
    int zi = ( si ) / ( xsize * ysize );
    int yi = ( si ) / ( xsize ) - ( zi * ysize );
    int xi = ( si ) % ( xsize );

    // allocate local memory for this work group
    __local int    slocal[1024];
    __local int    qlocal[1024];
    __local double vlocal[1024];

    // each worker sets a different index of the local memory to zero and waits
    vlocal[ get_local_id(0) ] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    // calculate how many pieces we can divide the total list of charges/positions into
    int num_loads = n / ( get_local_size(0) ) + 1;

    // start loading chunks of charges to calculate on
    for ( int load_number = 0; load_number < num_loads; load_number++ )
    {
        // each worker loads a different charge
        int load_id = get_local_size(0) * load_number + get_local_id(0);
        if ( load_id < n ) //number of charges
        {
            slocal[ get_local_id(0) ] = s[load_id];
            qlocal[ get_local_id(0) ] = q[load_id];
        }
        else
        {
            slocal[ get_local_id(0) ] = -1;
            qlocal[ get_local_id(0) ] = -1;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // each worker perfroms a different q/r calculation
        int sj = slocal[ get_local_id(0) ];
        if ( sj >= 0 )
        {
        int   zj = ( sj ) / ( xsize * ysize );
        int   yj = ( sj ) / ( xsize ) - ( zj * ysize );
        int   xj = ( sj ) % ( xsize );
        double r = ( xi - xj ) * ( xi - xj ) +
                   ( yi - yj ) * ( yi - yj ) +
                   ( zi - zj ) * ( zi - zj );

        // compute the interaction
        if ( r > 0 && r < c2 )
        {
            vlocal[ get_local_id(0) ] = vlocal[ get_local_id(0) ] + qlocal[ get_local_id(0) ] * rsqrt(r);
        }

        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if ( get_local_id(0) == 0 )
    {
        double v = 0;
        for ( int l = 0; l <  get_local_size(0); l++ )
        {
            v = v + vlocal[l];
        }
        //o[si] = v;
        o[ get_group_id(0) ] = prefactor * v;
    }
}

__kernel void gauss2( __global double *o, __global int *s, __global int *q, int n, int c2, __global int *w, int xsize, int ysize, double prefactor, double erffactor )
{
    // each worker of work group loads the same charge and site, using the "work group id"
    int qi = q[ get_group_id(0) ];
    int si = w[ get_group_id(0) ];

    // extract position from site using the grid dimensions
    int zi = ( si ) / ( xsize * ysize );
    int yi = ( si ) / ( xsize ) - ( zi * ysize );
    int xi = ( si ) % ( xsize );

    // allocate local memory for this work group
    __local int    slocal[1024];
    __local int    qlocal[1024];
    __local double vlocal[1024];

    // each worker sets a different index of the local memory to zero and waits
    vlocal[ get_local_id(0) ] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    // calculate how many pieces we can divide the total list of charges/positions into
    int num_loads = n / ( get_local_size(0) ) + 1;

    // start loading chunks of charges to calculate on
    for ( int load_number = 0; load_number < num_loads; load_number++ )
    {
        // each worker loads a different charge
        int load_id = get_local_size(0) * load_number + get_local_id(0);
        if ( load_id < n ) //number of charges
        {
            slocal[ get_local_id(0) ] = s[load_id];
            qlocal[ get_local_id(0) ] = q[load_id];
        }
        else
        {
            slocal[ get_local_id(0) ] = -1;
            qlocal[ get_local_id(0) ] = -1;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // each worker perfroms a different q/r calculation
        int sj = slocal[ get_local_id(0) ];
        if ( sj >= 0 )
        {
        int   zj = ( sj ) / ( xsize * ysize );
        int   yj = ( sj ) / ( xsize ) - ( zj * ysize );
        int   xj = ( sj ) % ( xsize );
        double r = ( xi - xj ) * ( xi - xj ) +
                   ( yi - yj ) * ( yi - yj ) +
                   ( zi - zj ) * ( zi - zj );

        // compute the interaction
        if ( r > 0 && r < c2 )
        {
	    r = sqrt(r);
            vlocal[ get_local_id(0) ] = vlocal[ get_local_id(0) ] + qlocal[ get_local_id(0) ] * erf(erffactor * r) / r;
        }

        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if ( get_local_id(0) == 0 )
    {
        double v = 0;
        for ( int l = 0; l <  get_local_size(0); l++ )
        {
            v = v + vlocal[l];
        }

        o[ get_group_id(0) ] = prefactor * v;
    }
}

// this is not used, was just fooling with images
__kernel void image( __write_only image2d_t img, __global double *o, int layer, double cmax, double cmin )
{
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    int i = get_global_id(0) + get_global_id(1) * get_global_size(0) + layer * get_global_size(0) * get_global_size(1);
    float4 val;
    double e = o[i];
    if ( e < 0 )
    {
       val.x = fabs(e / cmin); // green
       val.y = 0; // red
       val.z = 0; // alpha
       val.w = 0; // blue
    }
    else
    {
       val.x = 0; // green
       val.y = fabs(e / cmax); // red
       val.z = 0; // alpha
       val.w = 0; // blue
    }
    write_imagef(img, coord, val);
}
