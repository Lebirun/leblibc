#include <wchar.h>

size_t mbsnrtowcs(wchar_t *restrict wcs, const char **restrict src, size_t n, size_t wn, mbstate_t *restrict st)
{
	size_t l, cnt=0, n2;
	wchar_t *ws, wbuf[256];
	const char *s = *src;
	const char *tmp_s;

	if (!wcs) ws = wbuf, wn = sizeof wbuf / sizeof *wbuf;
	else ws = wcs;

	

	while ( s && wn && ( (n2=n/4)>=wn || n2>32 ) ) {
		if (n2>=wn) n2=wn;
		tmp_s = s;
		l = mbsrtowcs(ws, &s, n2, st);
		if (!(l+1)) {
			cnt = l;
			wn = 0;
			break;
		}
		if (ws != wbuf) {
			ws += l;
			wn -= l;
		}
		n = s ? n - (s - tmp_s) : 0;
		cnt += l;
	}
	if (s) while (wn && n) {
		l = mbrtowc(ws, s, n, st);
		if (l+2<=2) {
			if (!(l+1)) {
				cnt = l;
				break;
			}
			if (!l) {
				s = 0;
				break;
			}
			
			*(unsigned *)st = 0;
			break;
		}
		s += l; n -= l;
		
		ws++; wn--;
		cnt++;
	}
	if (wcs) *src = s;
	return cnt;
}
