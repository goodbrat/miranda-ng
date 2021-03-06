/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/


#ifndef __MIR_BUFFER_H__
# define __MIR_BUFFER_H__

#include <windows.h>

#include <m_variables.h>
#include <m_timezones.h>

template<class T>
inline size_t __blen(const T *str)
{
	return 0;
}

template<>
inline size_t __blen<char>(const char *str)
{
	return mir_strlen(str);
}

template<>
inline size_t __blen<wchar_t>(const wchar_t *str)
{
	return mir_wstrlen(str);
}

template<class T>
inline T * __bTranslate(const T *str)
{
	return 0;
}

template<>
inline char * __bTranslate<char>(const char *str)
{
	return Translate(str);
}

template<>
inline wchar_t * __bTranslate<wchar_t>(const wchar_t *str)
{
	return TranslateW(str);
}


template<class O, class D>
inline void __bcopy(D *dest, const O *orig, size_t len)
{
}

inline void __bcopy(char *dest, const char *orig, size_t len)
{
	strncpy(dest, orig, len);
}

inline void __bcopy(WCHAR *dest, const WCHAR *orig, size_t len)
{
	wcsncpy(dest, orig, len);
}

inline void __bcopy(WCHAR *dest, const char *orig, size_t len)
{
	MultiByteToWideChar(CallService("LangPack/GetCodePage", 0, 0), 0, orig, (int)len, dest, (int)len);
}

inline void __bcopy(char *dest, const WCHAR *orig, size_t len)
{
	WideCharToMultiByte(CallService("LangPack/GetCodePage", 0, 0), 0, orig, (int)len, dest, (int)len, nullptr, nullptr);
}



template<class T>
class Buffer
{
	public:
		size_t len;
		T *str;

		Buffer() : str(nullptr), size(0), len(0)
		{
			alloc(1);
			pack();
		}

		Buffer(T in) : str(nullptr), size(0), len(0)
		{
			if (in == NULL)
			{
				alloc(1);
				pack();
			}
			else
			{
				str = in;
				size = len = __blen(str);
			}
		}

		~Buffer()
		{
			free();
		}

		void pack()
		{
			if (str != nullptr)
				memset(&str[len], 0, sizeof(str[len]));
		}

		void alloc(size_t total)
		{
			if (total > size)
			{
				size = total + 256 - total % 256;
				if (str == nullptr)
					str = (T *) mir_alloc(size * sizeof(T));
				else
					str = (T *) mir_realloc(str, size * sizeof(T));
			}
		}

		void free()
		{
			if (str != nullptr)
			{
				mir_free(str);
				str = nullptr;
				len = size = 0;
			}
		}

		void clear()
		{
			len = 0;
			pack();
		}

		void append(T app)
		{
			alloc(len + 1 + 1);

			str[len] = app;
			len++;
			pack();
		}

		void appendn(size_t n, T app)
		{
			alloc(len + n + 1);

			for (; n > 0; n--)
			{
				str[len] = app;
				len++;
			}
			pack();
		}

		void append(const char *app, size_t appLen = -1)
		{
			if (app == nullptr)
				return;
			if (appLen == -1)
				appLen = __blen(app);

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app, appLen);
			len += appLen;
			pack();
		}

		void append(const WCHAR *app, size_t appLen = -1)
		{
			if (app == nullptr)
				return;
			if (appLen == -1)
				appLen = __blen(app);

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app, appLen);
			len += appLen;
			pack();
		}

		void append(const Buffer<char> &app)
		{
			if (app.str == nullptr)
				return;
			size_t appLen = app.len;

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app.str, appLen);
			len += appLen;
			pack();
		}

		void append(const Buffer<WCHAR> &app)
		{
			size_t appLen = app.len;

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app.str	, appLen);
			len += appLen;
			pack();
		}

		void appendPrintf(const T *app, ...)
		{
			size_t total = len + 512;
			alloc(total);

			va_list arg;
			va_start(arg, app);
			total = __bvsnprintf<T>(&str[len], size - len - 1, app, arg); //!!!!!!!!!!!!
			len += total;
			pack();
		}

		void insert(size_t pos, T *app, size_t appLen = -1)
		{
			if (pos > len)
				pos = len;

			if (appLen == -1)
				appLen = __blen(app);

			alloc(len + appLen + 1);

			if (pos < len)
				memmove(&str[pos + appLen], &str[pos], (len - pos) * sizeof(T));
			memmove(&str[pos], app, appLen * sizeof(T));

			len += appLen;
			pack();
		}

		void replace(size_t start, size_t end, T *app, size_t appLen = -1)
		{
			if (start > len)
				start = len;
			
			if (end > len)
				end = len;
			if (end < start)
				end = start;

			if (appLen == -1)
				appLen = __blen(app);

			size_t oldLen = end - start;
			if (oldLen < appLen)
				alloc(len + appLen - oldLen + 1);

			if (end < len && oldLen != appLen)
				memmove(&str[start + appLen], &str[end], (len - end) * sizeof(T));
			memmove(&str[start], app, appLen * sizeof(T));

			len += appLen - oldLen;
			pack();
		}

		void replaceAll(T find, T replace)
		{
			for(size_t i = 0; i < len; i++)
				if (str[len] == find)
					str[len] = replace;
			pack();
		}

		void translate()
		{
			if (str == nullptr || len == 0)
				return;

			str[len] = 0;
			T *tmp = __bTranslate(str);
			len = __blen(tmp);
			alloc(len + 1);
			memmove(str, tmp, len * sizeof(T));
			pack();
		}

		void reverse()
		{
			for(size_t i = 0; i < len/2; i++)
			{
				T tmp = str[i];
				str[i] = str[len-i-1];
				str[len-i-1] = tmp;
			}
		}

		T *appender(size_t appLen)
		{
			alloc(len + appLen + 1);
			T *ret = &str[len];
			len += appLen;
			return ret;
		}

		T *lock(size_t maxSize)
		{
			alloc(len + maxSize + 1);
			return &str[len];
		}

		void release()
		{
			len += max(__blen(&str[len]), size - len - 1);
		}

		T *detach()
		{
			T *ret = str;
			str = nullptr;
			len = 0;
			return ret;
		}

		void trimRight()
		{
			if (str == nullptr)
				return;

			int e;
			for(e = len-1; e >= 0 && (str[e] == (T)' '
									  || str[e] == (T)'\t'
									  || str[e] == (T)'\r'
									  || str[e] == (T)'\n'); e--) ;
			len = e+1;
			pack();
		}

		void trimLeft()
		{
			if (str == nullptr)
				return;

			int s;
			for(s = 0; str[s] == (T)' '
					   || str[s] == (T)'\t'
					   || str[s] == (T)'\r'
					   || str[s] == (T)'\n'; s++) ;
			if (s > 0)
			{
				memmove(str, &str[s], (len - s) * sizeof(T));
				len -= s;
			}
			pack();
		}

		void trim()
		{
			trimRight();
			trimLeft();
		}

		Buffer<T>& operator+=(const char *txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator+=(const WCHAR *txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator+=(const Buffer<T> &txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const char *txt)
		{
			clear();
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const WCHAR *txt)
		{
			clear();
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const Buffer<T> &txt)
		{
			clear();
			append(txt);
			return *this;
		}


	private:
		size_t size;
};

#endif // __MIR_BUFFER_H__
