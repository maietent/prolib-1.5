#include "stdafx.h"
#include "MBMatchAgent.h"
#include "MAgentConfig.h"

int main()
{
	MBMatchAgent* m_pMatchAgent = new MBMatchAgent;
	MAgentConfig* pAgentConfig = MAgentConfig::GetInstance();
	m_pMatchAgent->SetIP(pAgentConfig->GetIP());
	m_pMatchAgent->SetTCPPort(pAgentConfig->GetTCPPort());
	m_pMatchAgent->SetUDPPort(pAgentConfig->GetUDPPort());
	m_pMatchAgent->SetMatchServerIP(pAgentConfig->GetMatchServerIP());
	m_pMatchAgent->SetMatchServerTCPPort(pAgentConfig->GetMatchServerTCPPort());
	pAgentConfig->ReleaseInstance();

	if (m_pMatchAgent->Create(NULL)) {
		//m_pMatchAgent->ConnectToMatchServer(NULL, 0);
		for (;;) m_pMatchAgent->Run();
	}

	delete m_pMatchAgent;
	return 0;
}