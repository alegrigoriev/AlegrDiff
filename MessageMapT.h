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

#define BEGIN_MESSAGE_MAP_A(theClass, baseClass, _type) \
	template<_type A>    \
	const AFX_MSGMAP* PASCAL theClass<A>::GetThisMessageMap() \
		{ return &theClass<A>::messageMap; } \
	template<_type A>    \
	const AFX_MSGMAP* theClass<A>::GetMessageMap() const \
		{ return &theClass<A>::messageMap; } \
	template<_type A>    \
	AFX_COMDAT const AFX_MSGMAP theClass<A>::messageMap = \
	{ baseClass::GetThisMessageMap, theClass<A>::_messageEntries }; \
	template<_type A>    \
	AFX_COMDAT const AFX_MSGMAP_ENTRY theClass<A>::_messageEntries[] = \
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

#define BEGIN_MESSAGE_MAP_A(theClass, baseClass, _type) \
	template<_type A>    \
	const AFX_MSGMAP* theClass<A>::GetMessageMap() const \
		{ return &theClass<A>::messageMap; } \
	template<_type A>    \
	AFX_COMDAT const AFX_MSGMAP theClass<A>::messageMap = \
	{ &baseClass::messageMap, theClass<A>::_messageEntries }; \
	template<_type A>    \
	AFX_COMDAT const AFX_MSGMAP_ENTRY theClass<A>::_messageEntries[] = \
	{

#endif
