#pragma once

#define ISNULL(pt)			(!pt)
#define _countof(arr)		(sizeof(arr)/sizeof(arr[0]))

template<typename To, typename From>
inline To union_cast(From fr) throw(){
	union{
		From f;
		To t;
	} uc;
	uc.f = fr;
	return uc.t;
}
