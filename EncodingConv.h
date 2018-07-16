class EncodingConv
{
public:
	EncodingConv() : m_buf(NULL), m_wbuf(NULL), m_size(-1), m_wsize(-1) {}
	~EncodingConv()
	{
		if (m_buf)
			delete[] m_buf;
		if (m_wbuf)
			delete[] m_wbuf;
	}

	char* ToChar(int cp, LPCTSTR buf, int nlen=-1)
	{
		if (nlen<=0)
			nlen=_tcslen(buf);
		int nlen2=nlen<<1;
		if (m_size<nlen2)
		{
			if (m_buf)
				delete[] m_buf;
			m_size=((nlen2>>4)+1)<<4;
			m_buf=new char[m_size+1];
		}
		memset(m_buf, 0, sizeof(char)*(m_size+1));
		::WideCharToMultiByte(cp,0,buf,nlen,m_buf,m_size,NULL,NULL);
		return m_buf;
	}
	char* ToCharCopy(int cp, LPCTSTR buf, int nlen = -1)
	{
		if (nlen <= 0)
			nlen = _tcslen(buf);
		int nlen2 = ::WideCharToMultiByte(cp, 0, buf, nlen, NULL, 0, NULL, NULL);
		char* cbuf = (char*)malloc(nlen2+1);
		//memset(cbuf, 0, nlen2+1);
		nlen2=::WideCharToMultiByte(cp, 0, buf, nlen, cbuf, nlen2, NULL, NULL);
		cbuf[nlen2] = 0;
		return cbuf;
	}


	WCHAR* ToWChar(int cp, LPCSTR buf, int nlen=-1)
	{
		if (nlen<=0)
			nlen=strlen(buf);
		if (m_wsize<nlen)
		{
			if (m_wbuf)
				delete[] m_wbuf;
			m_wsize=((nlen>>3)+1)<<3;
			m_wbuf=new WCHAR[m_wsize+1];
		}
		memset(m_wbuf, 0, sizeof(WCHAR)*(m_wsize+1));
		::MultiByteToWideChar(cp,0,buf,nlen,m_wbuf,m_wsize);
		return m_wbuf;
	}

	WCHAR* ToWCharCopy(int cp, LPCSTR buf, int nlen = -1)
	{
		if (nlen <= 0)
			nlen = strlen(buf);
		int wsize= ::MultiByteToWideChar(cp, 0, buf, nlen, NULL, 0);
		WCHAR* wbuf= (WCHAR*)malloc((wsize+1)*sizeof(WCHAR));
		wsize=::MultiByteToWideChar(cp, 0, buf, nlen, wbuf, wsize);
		wbuf[wsize] = 0;
		return wbuf;
	}

	char* GetChar() { return m_buf; }
	WCHAR* GetWChar() { return m_wbuf; }

private:
	char* m_buf;
	WCHAR* m_wbuf;
	int m_size;
	int m_wsize;
};
