//----------------------------------------------------------------
//							TFile.h
//----------------------------------------------------------------

#include "CorePCH.h"
#include "TFile.h"

//---------------------------------------------------------------------
TFile::TFile(long type)
{
	m_bOpen = false;
	m_stream=NULL;
	m_StorageType = type; //0=file, 1=memory
	m_Position = m_Size = m_Edge = m_DontCloseOnExit = 0;
}

//---------------------------------------------------------------------
TFile::TFile(FILE* pFile)
{
	m_stream=pFile;
	m_StorageType = 0;
	m_Position = m_Size = m_Edge = 0;
	m_DontCloseOnExit = 1;
}

//---------------------------------------------------------------------
TFile::TFile(BYTE* pBuffer, size_t size)
{
	m_stream=pBuffer;
	m_StorageType = 1;
	m_Position = 0;
	m_Size = m_Edge = size;
	m_DontCloseOnExit = 1;
}

void TFile::Transfer(TFile& from)
{
	m_stream = from.m_stream;
	m_StorageType = from.m_StorageType;
	m_Position = from.m_Position;
	m_Size = from.m_Size;
	m_Edge = from.m_Edge;
	m_DontCloseOnExit = from.m_DontCloseOnExit;

	from.m_stream = NULL;
}
//---------------------------------------------------------------------
TFile::~TFile()
{
	Close();
}
//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
void* TFile::Open(const TCHAR* filename, const TCHAR* mode)
{
	if (m_stream)	return m_stream;
	

	errno_t err;

	switch(m_StorageType)
	{
		case 0:
		{
			
			if((err= tFOPEN_S((FILE**)&m_stream, filename, mode)==0))
			{
				m_bOpen = true;
				return m_stream;
			}

		}
		break;
		case 1:
		{
			m_Position = m_Size = m_Edge = 0;
			return m_stream=malloc(0);
		}
		break;
	}
	return 0;
}
//---------------------------------------------------------------------
long TFile::Close()
{
	if (m_DontCloseOnExit) return -1;
	if (m_stream==0) return false;
	switch(m_StorageType)
	{
	case 0:
		{
			m_bOpen = false;
			long lerr = fclose(static_cast<FILE*>(m_stream));
			m_stream=NULL;
			return lerr;
		}
	case 1:
		free(m_stream);
		m_stream=NULL;
		return 0;
	}
	return -1;
}

//---------------------------------------------------------------------
size_t TFile::Read(void *buffer, size_t size, size_t count)
{
	if (m_stream==NULL) return 0;
	switch(m_StorageType)
	{
	case 0:
		return fread(buffer, size, count, static_cast<FILE*>(m_stream));
	case 1:
		{
			if (buffer==NULL) return 0;
			if (m_Position > m_Size) return 0;

			size_t nCount = count*size;
			if (nCount == 0) return 0;
			
			size_t nRead;
			if (m_Position + nCount > m_Size)
				nRead = m_Size - m_Position;
			else
				nRead = nCount;

			memcpy(buffer, (static_cast<BYTE*>(m_stream)) + m_Position, nRead);
			m_Position += nRead;

			return nRead/size;
		}
	}
	return 0;
}

//---------------------------------------------------------------------
size_t TFile::Write(const void* buffer, size_t size, size_t count)
{
	if (m_stream==NULL) return 0;
	switch(m_StorageType)
	{
	case 0:
		return fwrite(buffer, size, count, static_cast<FILE*>(m_stream));
	case 1:
		{
			if (buffer==NULL) return 0;

			size_t nCount = count*size;
			if (nCount == 0) return 0;

			if (m_Position + nCount > m_Edge) Alloc(m_Position + nCount);

			memcpy((static_cast<BYTE*>(m_stream)) + m_Position, buffer, nCount);

			m_Position += nCount;

			if (m_Position > m_Size) m_Size = m_Position;
		}
		return count;
	}
	return 0;
}

//---------------------------------------------------------------------
long TFile::Seek(long offset, int origin)
{
	if (m_stream==NULL) return 0;
	switch(m_StorageType)
	{
		case 0:
			return static_cast<long>(fseek(static_cast<FILE*>(m_stream), offset, origin));
		case 1:
		{
			size_t lNewPos = m_Position;

			if (origin == SEEK_SET)		 lNewPos = static_cast<size_t>(offset);
			else if (origin == SEEK_CUR) lNewPos += static_cast<size_t>(offset);
			else if (origin == SEEK_END) lNewPos = m_Size + static_cast<size_t>(offset);
			else return -1;

			if (lNewPos < 0) return -1;

			m_Position = lNewPos;
			return 0;
		}
	}
	return -1;
}

//---------------------------------------------------------------------
LONG_PTR TFile::Tell()
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType){
	case 0:
#ifdef WIN64
		return static_cast<LONG_PTR>(_ftelli64(static_cast<FILE*>(m_stream)));
#else
		return static_cast<LONG_PTR>(ftell(static_cast<FILE*>(m_stream)));
#endif
	case 1:
		return static_cast<LONG_PTR>(m_Position);
	}
	return -1;
}

//---------------------------------------------------------------------
long TFile::GetPos(fpos_t *pos)
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType){
	case 0:
		fgetpos(static_cast<FILE*>(m_stream), pos);
		return 0;
	case 1:
		*pos = static_cast<fpos_t>(m_Position);
		return 0;
	}
	return -1;
}

//---------------------------------------------------------------------
size_t TFile::Size()
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType){
	case 0:
		{
			LONG_PTR pos,size;
#ifdef WIN64
			pos = _ftelli64(static_cast<FILE*>(m_stream));
			_fseeki64(static_cast<FILE*>(m_stream),0,SEEK_END);
			size = _ftelli64(static_cast<FILE*>(m_stream));
			_fseeki64(static_cast<FILE*>(m_stream),pos,SEEK_SET);
#else
			pos = ftell(static_cast<FILE*>(m_stream));
			fseek(static_cast<FILE*>(m_stream),0,SEEK_END);
			size = ftell(static_cast<FILE*>(m_stream));
			fseek(static_cast<FILE*>(m_stream),pos,SEEK_SET);
#endif
			return static_cast<size_t>(size);
		}
	case 1:
		return m_Size;
	}
	return -1;
}

//---------------------------------------------------------------------
long TFile::Flush()
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType){
	case 0:
		return static_cast<long>(fflush(static_cast<FILE*>(m_stream)));
	case 1:
		return 0;
	}
	return -1;
}

//---------------------------------------------------------------------
long TFile::PutC(unsigned char c)
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType)
	{
	case 0:
		return static_cast<long>(fputc(c, static_cast<FILE*>(m_stream)));
	case 1:
		{
			if (m_Position + 1 > m_Edge) Alloc(m_Position + 1);

			memcpy((static_cast<BYTE*>(m_stream)) + m_Position, &c, 1);

			m_Position += 1;

			if (m_Position > m_Size) m_Size = m_Position;
			
			return static_cast<long>(c);
		}
	}
	return -1;
}

//---------------------------------------------------------------------
long TFile::GetC()
{
	if (m_stream==NULL) return -1;
	switch(m_StorageType)
	{
	case 0:
		return static_cast<long>(getc(static_cast<FILE*>(m_stream)));
	case 1:
		return static_cast<long>(*(static_cast<BYTE*>((static_cast<BYTE*>(m_stream)) + m_Position)));
	}
	return -1;
}

//---------------------------------------------------------------------
void* TFile::Alloc(size_t dwNewLen)
{
	switch(m_StorageType)
	{
	case 0:
		return m_stream;
	case 1:
		if (dwNewLen > m_Edge)
		{
			// determine new buffer size
			size_t dwNewBufferSize = m_Edge;
			while (dwNewBufferSize < dwNewLen) dwNewBufferSize += 4096;

			// allocate new buffer
			if (m_stream == NULL) m_stream = malloc(dwNewBufferSize);
			else	m_stream = realloc(m_stream, dwNewBufferSize);

			m_Edge = dwNewBufferSize;
		}
		return m_stream;
	}
	return m_stream;
}
//////////////////////////////////////////////////////////
void TFile::Free()
{
	Close();
}
//////////////////////////////////////////////////////////
