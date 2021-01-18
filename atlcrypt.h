// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLCRYPT_H__
#define __ATLCRYPT_H__

#pragma once

#include <atlchecked.h>
#include <wincrypt.h>


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

class CCryptKey;

class CCryptProv
{
protected:
	HCRYPTPROV m_hProv;

public:
	CCryptProv() noexcept;
	CCryptProv( const CCryptProv& prov ) noexcept;
	explicit CCryptProv( HCRYPTPROV hProv, BOOL bTakeOwnership = FALSE ) noexcept;
	~CCryptProv() noexcept;

	CCryptProv& operator=( const CCryptProv& prov ) noexcept;

	HRESULT AddRef() noexcept;
	void Attach( HCRYPTPROV hProv, BOOL bTakeOwnership = FALSE ) noexcept;
	HCRYPTPROV Detach() noexcept;
	HRESULT Release() noexcept;


	HRESULT Initialize(DWORD dwProviderType = PROV_RSA_FULL,
						LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
						DWORD dwFlags = 0) noexcept;
	HRESULT InitVerifyContext(DWORD dwProviderType = PROV_RSA_FULL,
							LPCTSTR szProvider = MS_DEF_PROV, DWORD dwFlags = 0) noexcept;
	HRESULT InitCreateKeySet(DWORD dwProviderType = PROV_RSA_FULL,
							LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
							DWORD dwFlags = 0) noexcept;

	HRESULT DeleteKeySet(DWORD dwProviderType = PROV_RSA_FULL,
						LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
						DWORD dwFlags = 0) noexcept;

	HRESULT Uninitialize() noexcept;

	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) noexcept;
	HRESULT SetParam( DWORD dwParam, BYTE* pbData, DWORD dwFlags = 0) noexcept;
	HRESULT GetName(__out_ecount_part_z(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) noexcept;
	HRESULT GetContainer(__out_ecount_part_z(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) noexcept;
	HRESULT GetImpType(DWORD * pdwImpType) noexcept;
	HRESULT GetVersion(DWORD * pdwVersion) noexcept;
	HRESULT GetProvType(DWORD * pdwType) noexcept;
	HRESULT GetSecurityDesc(SECURITY_INFORMATION * pSecInfo) noexcept;
	HRESULT SetSecurityDesc(SECURITY_INFORMATION SecInfo) noexcept;

	HRESULT GenRandom(ULONG nLength, BYTE* pbBuffer ) noexcept;

	inline HCRYPTPROV GetHandle() noexcept
	{
		return m_hProv;
	}
}; // class CCryptProv


// class CCryptHash
// Provides base functionality of hashes.
class CCryptHash
{
protected:
	HCRYPTHASH m_hHash;

public:
	CCryptHash() noexcept;
	CCryptHash( const CCryptHash& hash ) noexcept;
	explicit CCryptHash( HCRYPTHASH hHash, BOOL bTakeOwnership = FALSE ) noexcept;
	~CCryptHash() noexcept;

	void Attach( HCRYPTHASH hHash, BOOL bTakeOwnership = FALSE ) noexcept;
	void Destroy() noexcept;
	HCRYPTHASH Detach() noexcept;
	HCRYPTHASH Duplicate() const noexcept;

	HRESULT Uninitialize() noexcept;
	HRESULT Detach(HCRYPTHASH * phHash) noexcept;
	HRESULT AddData(const BYTE * pbData, DWORD dwDataLen, DWORD dwFlags = 0) noexcept;
	HRESULT AddString(LPCTSTR szData, DWORD dwFlags = 0) noexcept;
	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) noexcept;
	HRESULT SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags = 0) noexcept;
	HRESULT GetAlgId(ALG_ID * pAlgId) noexcept;
	HRESULT GetSize(DWORD * pdwSize) noexcept;
	HRESULT GetValue(BYTE * pBuf, DWORD * pdwSize) noexcept;
	HRESULT SetValue(BYTE * pBuf) noexcept;
	HRESULT Sign(
				BYTE * pbSignature,
				DWORD * pdwSigLen,
				DWORD dwFlags = 0,
				DWORD dwKeySpec = AT_SIGNATURE) noexcept;
	HRESULT VerifySignature(
							const BYTE * pbSignature,
							DWORD pdwSignLen,
							CCryptKey &PubKey,
							DWORD dwFlags = 0) noexcept;

	inline HCRYPTHASH GetHandle() noexcept
	{
		return m_hHash;
	}
	static CCryptHash EmptyHash;

}; // class CCryptHash

// class CCryptKey
// Provides the functionality for cryptographic keys, i.e. encrypting, decrypting.
class CCryptKey
{
protected:
	HCRYPTKEY m_hKey;

public:
	CCryptKey() noexcept;
	CCryptKey( const CCryptKey& key ) noexcept;
	explicit CCryptKey( HCRYPTKEY hKey, BOOL bTakeOwnership = FALSE ) noexcept;
	~CCryptKey() noexcept;

	void Attach( HCRYPTKEY hKey, BOOL bTakeOwnership = FALSE ) noexcept;
	void Destroy() noexcept;
	HCRYPTKEY Detach() noexcept;
	HCRYPTKEY Duplicate() const noexcept;

	HRESULT Uninitialize() noexcept;
	HRESULT Encrypt(
					BOOL final,
					BYTE * pbData,
					DWORD * pdwDataLen,
					DWORD dwBufLen,
					CCryptHash &Hash = CCryptHash::EmptyHash) noexcept;

	HRESULT Encrypt(
					const BYTE * pbPlainText,
					DWORD dwPlainTextLen,
					BYTE * pbCipherText,
					DWORD * pdwCipherTextLen,
					CCryptHash &Hash = CCryptHash::EmptyHash) noexcept;

	HRESULT Decrypt(
					BOOL final,
					BYTE * pbData,
					DWORD * pdwDataLen,
					CCryptHash &Hash = CCryptHash::EmptyHash) noexcept;
	HRESULT Decrypt(
					const BYTE * pbCipherText,
					DWORD dwCipherTextLen,
					BYTE * pbPlainText,
					DWORD * pdwPlainTextLen,
					CCryptHash &Hash = CCryptHash::EmptyHash) noexcept;
	HRESULT EncryptString(
						LPCTSTR szPlainText,
						BYTE * pbCipherText,
						DWORD * pdwCipherTextLen,
						CCryptHash &Hash = CCryptHash::EmptyHash) noexcept;
	HRESULT ExportSimpleBlob(
							CCryptKey &ExpKey,
							DWORD dwFlags,
							BYTE * pbData,
							DWORD * pdwDataLen) noexcept;
	HRESULT ExportPublicKeyBlob(
								CCryptKey &ExpKey,
								DWORD dwFlags,
								BYTE * pbData,
								DWORD * pdwDataLen) noexcept;
	HRESULT ExportPrivateKeyBlob(
								CCryptKey &ExpKey,
								DWORD dwFlags,
								BYTE * pbData,
								DWORD * pdwDataLen) noexcept;
	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) noexcept;
	HRESULT SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags = 0) noexcept;
	HRESULT GetAlgId(ALG_ID * pAlgId) noexcept;
	HRESULT SetAlgId(ALG_ID AlgId, DWORD dwFlags) noexcept;
	HRESULT GetBlockLength(DWORD * pdwBlockLen) noexcept;
	HRESULT GetKeyLength(DWORD * pdwKeyLen) noexcept;
	HRESULT GetSalt(BYTE * pbSalt, DWORD * pdwLength) noexcept;
	HRESULT SetSalt(BYTE * pbSalt) noexcept;
	HRESULT SetSaltEx(_CRYPTOAPI_BLOB * pBlobSalt) noexcept;
	HRESULT GetPermissions(DWORD * pdwPerms) noexcept;
	HRESULT SetPermissions(DWORD dwPerms) noexcept;
	HRESULT GetP(BYTE * pbP, DWORD * pdwLength) noexcept;
	HRESULT SetP(_CRYPTOAPI_BLOB * pBlobP) noexcept;
	HRESULT SetP(BYTE * pbP, DWORD dwLength) noexcept;
	HRESULT GetQ(BYTE * pbQ, DWORD * pdwLength) noexcept;
	HRESULT SetQ(_CRYPTOAPI_BLOB * pBlobQ) noexcept;
	HRESULT SetQ(BYTE * pbQ, DWORD dwLength) noexcept;
	HRESULT GetG(BYTE * pbG, DWORD * pdwLength) noexcept;
	HRESULT SetG(_CRYPTOAPI_BLOB * pBlobG) noexcept;
	HRESULT SetG(BYTE * pbG, DWORD dwLength) noexcept;
	HRESULT SetX() noexcept;
	HRESULT GetEffKeyLen(DWORD * pdwEffKeyLen) noexcept;
	HRESULT SetEffKeyLen(DWORD dwEffKeyLen) noexcept;
	HRESULT GetPadding(DWORD * pdwPadding) noexcept;
	HRESULT SetPadding(DWORD dwPadding) noexcept;
	HRESULT GetIV(BYTE * pbIV, DWORD * pdwLength) noexcept;
	HRESULT SetIV(BYTE * pbIV) noexcept;
	HRESULT GetMode(DWORD * pdwMode) noexcept;
	HRESULT SetMode(DWORD dwMode) noexcept;
	HRESULT GetModeBits(DWORD * pdwModeBits) noexcept;
	HRESULT SetModeBits(DWORD dwModeBits) noexcept;

	inline HCRYPTKEY GetHandle() noexcept
	{
		return m_hKey;
	}

	static CCryptKey EmptyKey;
}; // class CCryptKey



// Specific instances of Keys and Hashes

// class CCryptDerivedKey
// A key that is derived from a hashed password.  Two keys derived
// from the same password will be identical.
class CCryptDerivedKey : public CCryptKey
{
public:
	HRESULT Initialize(
						CCryptProv &Prov,
						CCryptHash &Hash,
						ALG_ID algid = CALG_RC4,
						DWORD dwFlags = CRYPT_EXPORTABLE) noexcept;
}; // class CCryptDerivedKey

// class CCryptRandomKey
// A randomly generated key.  Can be used internally by a program
// to protect data during execution, or can be exported with Crypt.Export
//
// Currently it is possible to pass in AT_KEYEXCHANGE or AT_SIGNATURE
// for algid, but these two will generate keys for the current key set, and
// the resulting handle can only be used for exporting and importing keys or
// signing hashes.
class CCryptRandomKey : public CCryptKey
{
public:
	HRESULT Initialize(
						CCryptProv &Prov,
						ALG_ID algid = CALG_RC4,
						DWORD dwFlags = CRYPT_EXPORTABLE) noexcept;
}; // class CCryptRandomKey

// class CCryptUserExKey
// Obtains the user's key exchange key pair.
class CCryptUserExKey : public CCryptKey
{
public:
	HRESULT Initialize(CCryptProv &Prov) noexcept;
	HRESULT Create(CCryptProv &Prov) noexcept;
}; // class CCryptUserExKey

// class CCryptUserSigKey
// Obtains the user's signature key pair
class CCryptUserSigKey : public CCryptKey
{
public:
	HRESULT Initialize(CCryptProv &Prov) noexcept;
	HRESULT Create(CCryptProv &Prov) noexcept;
}; // class CCryptUserSigKey

// class CCryptImportKey
// Forms a key from an imported key blob
class CCryptImportKey : public CCryptKey
{
public:
	HRESULT Initialize(
						CCryptProv &Prov,
						BYTE * pbData,
						DWORD dwDataLen,
						CCryptKey &PubKey,
						DWORD dwFlags) noexcept;
}; // class CCryptImportKey


// class CCryptHash
// A generic hash that may or may not take a key.
class CCryptKeyedHash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, ALG_ID Algid, CCryptKey &Key, DWORD dwFlags) noexcept;
}; // class CCryptKeyedHash

// class CCryptMD5Hash
// RSA's MD5 hash (RSA's most recent hash as of 9/7/99);
class CCryptMD5Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) noexcept;
}; // class CCryptMD5Hash

// class CCryptMD4Hash
// RSA's MD4 hash
class CCryptMD4Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) noexcept;
}; // class CCryptMD4Hash


// class CCryptMD2Hash
// RSA's MD2 hash
class CCryptMD2Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) noexcept;
}; // class CCryptMD2Hash


// class CCryptSHAHash
// The Secure Hash Algorithm hash, from NIST and NSA.  Technically, SHA-1.
class CCryptSHAHash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) noexcept;
}; // class CCryptSHAHash

// The Secure Hash Algorithm, from NIST and NSA.  Identical to CCryptSHA
typedef CCryptSHAHash CCryptSHA1Hash;


// class CCryptHMACHash
// Hash-base Message Authentication Code keyed hash
class CCryptHMACHash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) noexcept;
}; // class CCryptHMACHash

// class CCryptMACHash
// Message Authentication Code keyed hash.  Believed to be less secure than HMAC
class CCryptMACHash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) noexcept;
}; // class CCryptMACHash

// class CCryptSSL3SHAMD5Hash
// Hash algorithm used by Secure Socket Layer
class CCryptSSL3SHAMD5Hash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) noexcept;
}; // class CCryptSSl3SHAMD5Hash

}; // namespace ATL


#include "atlcrypt.inl"
#pragma pack(pop)
#endif  // __ATLCRYPT_H__
