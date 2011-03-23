/*
 * io/extent_file.cc
 *
 *  Created on: Mar 3, 2011
 *      Author: max
 */

#include "../first.hh"

#include <cerrno>            // for errno, ENOMEM, EINVAL, EFBIG
#include <cstdio>            // for FILE

#include "../types.hh"       // for ft_off
#include "../extent.hh"      // for ft_extent<T>
#include "../vector.hh"      // for ft_vector<T>
#include "extent_file.hh"    // for ff_read_extents_file()


FT_IO_NAMESPACE_BEGIN

/**
 * load file blocks allocation map (extents) previously saved into specified file
 * and appends them to ret_container (retrieves also user_data)
 * in case of failure returns errno-compatible error code, and ret_list contents will be UNDEFINED.
 *
 * implementation: simply reads the list of triplets (physical, logical, length)
 * stored in the stream as decimal numbers
 */
int ff_load_extents_file(FILE * f, ft_vector<ft_uoff> & ret_list, ft_uoff & ret_block_size_bitmask)
{
    {
        char header[200];
        for (ft_size i = 0; i < 6; i++)
            fgets(header, sizeof(header), f);
    }

    ft_ull physical, logical, length = 0, user_data;
    if (fscanf(f, "count %"FS_ULL"\n", & length) != 1)
        return EPROTO;

    ft_uoff block_size_bitmask = ret_block_size_bitmask;
    ft_size i = ret_list.size(), n = (ft_size) length;

    ret_list.resize(n += i);
    for (; i < n; i++) {
        if (fscanf(f, "%"FS_ULL" %"FS_ULL" %"FS_ULL" %"FS_ULL"\n", &physical, &logical, &length, &user_data) != 4)
            return EPROTO;

        ft_extent<ft_uoff> & extent = ret_list[i];

        block_size_bitmask |=
            (extent.physical() = (ft_uoff) physical) |
            (extent.logical()  = (ft_uoff) logical) |
            (extent.length()   = (ft_uoff) length);

        extent.user_data() = (ft_size) user_data;
    }
    ret_block_size_bitmask = block_size_bitmask;
    return 0;
}


/**
 * writes file blocks allocation map (extents) to specified FILE (stores also user_data)
 * in case of failure returns errno-compatible error code.
 *
 * implementation: simply writes the list of triplets (physical, logical, length)
 * into the FILE as decimal numbers
 */
int ff_save_extents_file(FILE * f, const ft_vector<ft_uoff> & extent_list)
{
    fprintf(f, "%s",
            "################################################################################\n"
            "######################  DO NOT EDIT THIS FILE ! ################################\n"
            "################################################################################\n"
            "## This file was automatically generated by fstransform,              ##########\n"
            "## and any change you may do will be overwritten upon next execution. ##########\n"
            "################################################################################\n");
    fprintf(f, "count %"FS_ULL"\n", (ft_ull) extent_list.size());
    fprintf(f, "physical\tlogical\tlength\tuser_data\n");

    ft_vector<ft_uoff>::const_iterator iter = extent_list.begin(), end = extent_list.end();
    for (; iter != end; ++iter) {
        const ft_extent<ft_uoff> & extent = *iter;
        if (fprintf(f, "%"FS_ULL"\t%"FS_ULL"\t%"FS_ULL"\t%"FS_ULL"\n",
                    (ft_ull)extent.physical(), (ft_ull)extent.logical(), (ft_ull)extent.length(), (ft_ull)extent.user_data())
                    <= 0)
            break;
    }
    return iter == end ? 0 : errno;
}


FT_IO_NAMESPACE_END
