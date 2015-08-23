//------------------------------------------------------------------------------------
//										TFile.h
//------------------------------------------------------------------------------------



#ifndef __TFILE__H__
#define __TFILE__H__



#include "../lib_Common/CommonPCH.h"
#include <windows.h>
#include <stdio.h>

class TFile
{
public:
	TFile(long type = 0);
	TFile(FILE* pFile);
	TFile(BYTE* pBuffer, size_t size);
	~TFile();
	void*	Open(const TCHAR* filename, const TCHAR* mode);
	long	Close();
	size_t	Read(void *buffer, size_t size, size_t count);
	size_t	Write(const void *buffer, size_t size, size_t count);
	long	Seek(long offset, int origin);
	LONG_PTR	Tell();
	size_t	Size();
	long	Flush();
	long	PutC(unsigned char c);
	long	GetC();
	long	GetPos(fpos_t *pos);
	void*	GetStream() const {return m_stream;}
	void*	Alloc(size_t nBytes);
	void	Free();
	void	Transfer(TFile &from);

	bool	isOpen(){ return m_bOpen; }
	

protected:
	void	*m_stream;	//file or memory pointer or whatever
	long	m_StorageType; //type (0=file, 1=memory, ...)
	size_t	m_Position;	//current position
	size_t	m_Size;		//file size
	size_t	m_Edge;		//buffer size
	size_t	m_DontCloseOnExit;
	bool	m_bOpen;

};

#endif //__xfile_h