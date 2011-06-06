#include "SockMgr.h"

static CSockMgr s_SockMgr;
CSockMgr* g_pSockMgr = &s_SockMgr;

CSockMgr::CSockMgr( void ) :
	m_Worker(m_IOService)
{
}

CSockMgr::~CSockMgr( void )
{
	std::vector<GLSock::ISock*>::iterator itr;

	m_Mutex.lock();

	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		(*itr)->Destroy();
	}
	
	m_Mutex.unlock();

	for(;;)
	{
		// TODO: Use scoped lock maybe?

		m_Mutex.lock();
		if( m_vecSocks.empty() )
		{
			m_Mutex.unlock();
			break;
		}
		m_Mutex.unlock();

		// Poll until the socks have been cleaned up.
		m_IOService.poll_one();
	}
}

/*
bool CSockMgr::StartThread()
{
	m_Thread = boost::thread( boost::bind(&boost::asio::io_service::run, &m_IOService) );
	return true;
}

bool CSockMgr::StopThread()
{
	m_IOService.stop();
	// m_IOService.reset();

	if( m_Thread.joinable() )
		m_Thread.join();

	return true;
}
*/

GLSock::ISock* CSockMgr::CreateAcceptorSock(lua_State* L)
{
	GLSock::CSockAcceptor* pSock = new GLSock::CSockAcceptor(m_IOService, L);
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::ISock* CSockMgr::CreateTCPSock( lua_State* L )
{
	GLSock::CSockTCP* pSock = new GLSock::CSockTCP(m_IOService, L);
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::ISock* CSockMgr::CreateUDPSock( lua_State* L )
{
	GLSock::CSockUDP* pSock = new GLSock::CSockUDP(m_IOService, L);
	m_vecSocks.push_back(pSock);

	return pSock;
}

bool CSockMgr::RemoveSock( GLSock::ISock* pSock )
{
	std::vector<GLSock::ISock*>::iterator itr;
	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		if( *itr == pSock )
		{
			m_vecSocks.erase(itr);
			return true;
		}
	}

	return false;
}

bool CSockMgr::ValidHandle( GLSock::ISock* pSock )
{
	std::vector<GLSock::ISock*>::iterator itr;
	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		if( *itr == pSock )
			return true;
	}

	return false;
}

/*
CMutex& CSockMgr::Mutex()
{
	return m_Mutex;
}
*/