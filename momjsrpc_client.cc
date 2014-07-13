// file momjsprc_client.cc

/**   Copyright (C)  2014 Free Software Foundation, Inc.
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.

    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

//// See http://www.tntnet.org/howto/jsonrpc-howto.html
#include <iostream>
#include <cxxtools/arg.h>
#include <cxxtools/remoteprocedure.h>
#include <cxxtools/json/rpcclient.h>
#include <cxxtools/serializationinfo.h>

struct momcli_status_st
{
    std::string mcl_timestamp;
    std::string mcl_lastgitcommit;
    double mcl_elapsedtime;
    double mcl_cputime;
    momcli_status_st() : mcl_timestamp(), mcl_lastgitcommit(), mcl_elapsedtime(0.0), mcl_cputime(0.0) {};
};


void operator>>= (const cxxtools::SerializationInfo& si,  momcli_status_st& mystatus)
{
    si.getMember("timestamp") >>= mystatus.mcl_timestamp;
    si.getMember("lastgitcommit") >>= mystatus.mcl_lastgitcommit;
    si.getMember("elapsedtime") >>= mystatus.mcl_elapsedtime;
    si.getMember("cputime") >>= mystatus.mcl_cputime;
}

int main (int argc, char**argv)
{
    try
        {
            cxxtools::Arg<std::string> ip(argc, argv, 'i', "localhost");
            cxxtools::Arg<unsigned short> port(argc, argv, 'p', 8086);
            cxxtools::json::RpcClient client(ip, port);
            cxxtools::RemoteProcedure<momcli_status_st> rpc_status(client, "state");
            momcli_status_st status = rpc_status();
            std::cout << "mominmelt timestamp:" << status.mcl_timestamp
                      << " lastgitcommit:" << status.mcl_lastgitcommit
                      << " elapsedtime:" << status.mcl_elapsedtime
                      << " cputime:" << status.mcl_cputime
                      << std::endl;
            exit (EXIT_SUCCESS);
        }
    catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
}
