// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once

#ifdef _AFXDLL
#define BEGIN_MESSAGE_MAP_T(theClass, baseClass) \
	template<typename T>    \
	const AFX_MSGMAP* PASCAL theClass<T>::GetThisMessageMap() \
		{ return &theClass<T>::messageMap; } \
	template<typename T>    \
	const AFX_MSGMAP* theClass<T>::GetMessageMap() const \
		{ return &theClass<T>::messageMap; } \
	template<typename T>    \
	AFX_COMDAT const AFX_MSGMAP theClass<T>::messageMap = \
	{ baseClass::GetThisMessageMap, theClass<T>::_messageEntries }; \
	template<typename T>    \
	AFX_COMDAT const AFX_MSGMAP_ENTRY theClass<T>::_messageEntries[] = \
	{

#else
#define BEGIN_MESSAGE_MAP_T(theClass, baseClass) \
	template<typename T>    \
	const AFX_MSGMAP* theClass<T>::GetMessageMap() const \
		{ return &theClass<T>::messageMap; } \
	template<typename T>    \
	AFX_COMDAT const AFX_MSGMAP theClass<T>::messageMap = \
	{ &baseClass::messageMap, theClass<T>::_messageEntries }; \
	template<typename T>    \
	AFX_COMDAT const AFX_MSGMAP_ENTRY theClass<T>::_messageEntries[] = \
	{

#endif
