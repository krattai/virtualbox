/* -*- indent-tabs-mode: nil; -*- */
#include <VBox/com/string.h>
#include <VBox/com/ptr.h>


#ifdef RT_OS_OS2
# include <sys/socket.h>
typedef int socklen_t;
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <iprt/assert.h>
#include <iprt/err.h>
#include <iprt/file.h>
#include <iprt/critsect.h>

#include <VBox/log.h>

#include <string>

#include "HostDnsService.h"
#include "../../Devices/Network/slirp/resolv_conf_parser.h"


struct HostDnsServiceResolvConf::Data
{
    Data(const char *fileName):resolvConfFilename(fileName){};

    std::string resolvConfFilename;
};

const std::string& HostDnsServiceResolvConf::resolvConf() const
{
    return m->resolvConfFilename;
}


HostDnsServiceResolvConf::~HostDnsServiceResolvConf()
{
    if (m)
    {
        delete m;
        m = NULL;
    }
}

HRESULT HostDnsServiceResolvConf::init(const char *aResolvConfFileName)
{
    m = new Data(aResolvConfFileName);

    HostDnsMonitor::init();

    readResolvConf();

    return S_OK;
}


HRESULT HostDnsServiceResolvConf::readResolvConf()
{
    struct rcp_state st;
    
    st.rcps_flags = RCPSF_NO_STR2IPCONV; 
    int rc = rcp_parse(&st, m->resolvConfFilename.c_str());
    if (rc == -1)
        return S_OK;

    HostDnsInformation info;
    for (unsigned i = 0; i != st.rcps_num_nameserver; ++i)
    {
        AssertBreak(st.rcps_str_nameserver[i]);
        info.servers.push_back(st.rcps_str_nameserver[i]);
    }
    
    if (st.rcps_domain)
        info.domain = st.rcps_domain;

    for (unsigned i = 0; i != st.rcps_num_searchlist; ++i)
    {
        AssertBreak(st.rcps_searchlist[i]);
        info.searchList.push_back(st.rcps_searchlist[i]);
    }
    setInfo(info);

    return S_OK;
}
