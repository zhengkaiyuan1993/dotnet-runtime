// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include "pal_ssl.h"
#include <dlfcn.h>

#include "coretls_structs.h"

SSLContextRef AppleCryptoNative_SslCreateContext(int32_t isServer)
{
    if (isServer != 0 && isServer != 1)
        return NULL;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SSLCreateContext(NULL, isServer ? kSSLServerSide : kSSLClientSide, kSSLStreamType);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetConnection(SSLContextRef sslContext, SSLConnectionRef sslConnection)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SSLSetConnection(sslContext, sslConnection);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetAcceptClientCert(SSLContextRef sslContext)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // NULL and other illegal values are handled by the underlying API
    return SSLSetClientSideAuthenticate(sslContext, kTryAuthenticate);
#pragma clang diagnostic pop
}

static SSLProtocol PalSslProtocolToSslProtocol(PAL_SslProtocol palProtocolId)
{
    switch (palProtocolId)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case PAL_SslProtocol_Tls13:
            return kTLSProtocol13;
        case PAL_SslProtocol_Tls12:
            return kTLSProtocol12;
        case PAL_SslProtocol_Tls11:
            return kTLSProtocol11;
        case PAL_SslProtocol_Tls10:
            return kTLSProtocol1;
        case PAL_SslProtocol_Ssl3:
            return kSSLProtocol3;
        case PAL_SslProtocol_Ssl2:
            return kSSLProtocol2;
        case PAL_SslProtocol_None:
        default:
            return kSSLProtocolUnknown;
#pragma clang diagnostic pop
    }
}

int32_t AppleCryptoNative_SslSetMinProtocolVersion(SSLContextRef sslContext, PAL_SslProtocol sslProtocol)
{
    SSLProtocol protocol = PalSslProtocolToSslProtocol(sslProtocol);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (protocol == kSSLProtocolUnknown)
        return errSecParam;

    // NULL and other illegal values are handled by the underlying API
    return SSLSetProtocolVersionMin(sslContext, protocol);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetMaxProtocolVersion(SSLContextRef sslContext, PAL_SslProtocol sslProtocol)
{
    SSLProtocol protocol = PalSslProtocolToSslProtocol(sslProtocol);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (protocol == kSSLProtocolUnknown)
        return errSecParam;

    // NULL and other illegal values are handled by the underlying API
    return SSLSetProtocolVersionMax(sslContext, protocol);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslCopyCertChain(SSLContextRef sslContext, SecTrustRef* pChainOut, int32_t* pOSStatus)
{
    if (pChainOut != NULL)
        *pChainOut = NULL;
    if (pOSStatus != NULL)
        *pOSStatus = noErr;

    if (sslContext == NULL || pChainOut == NULL || pOSStatus == NULL)
        return -1;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    *pOSStatus = SSLCopyPeerTrust(sslContext, pChainOut);
#pragma clang diagnostic pop
    return *pOSStatus == noErr;
}

int32_t
AppleCryptoNative_SslCopyCADistinguishedNames(SSLContextRef sslContext, CFArrayRef* pArrayOut, int32_t* pOSStatus)
{
    if (pArrayOut != NULL)
        *pArrayOut = NULL;
    if (pOSStatus != NULL)
        *pOSStatus = noErr;

    if (sslContext == NULL || pArrayOut == NULL || pOSStatus == NULL)
        return -1;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    *pOSStatus = SSLCopyDistinguishedNames(sslContext, pArrayOut);
#pragma clang diagnostic pop

    return *pOSStatus == noErr;
}

static int32_t SslSetSessionOption(SSLContextRef sslContext,
                                   SSLSessionOption option,
                                   int32_t value,
                                   int32_t* pOSStatus)
{
    if (sslContext == NULL)
        return -1;

    if (value != 0 && value != 1)
        return -2;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    *pOSStatus = SSLSetSessionOption(sslContext, option, !!value);
#pragma clang diagnostic pop

    return *pOSStatus == noErr;
}

int32_t AppleCryptoNative_SslSetBreakOnServerAuth(SSLContextRef sslContext, int32_t setBreak, int32_t* pOSStatus)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SslSetSessionOption(sslContext, kSSLSessionOptionBreakOnServerAuth, setBreak, pOSStatus);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetBreakOnCertRequested(SSLContextRef sslContext, int32_t setBreak, int32_t* pOSStatus)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SslSetSessionOption(sslContext, kSSLSessionOptionBreakOnCertRequested, setBreak, pOSStatus);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetBreakOnClientAuth(SSLContextRef sslContext, int32_t setBreak, int32_t* pOSStatus)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SslSetSessionOption(sslContext, kSSLSessionOptionBreakOnClientAuth, setBreak, pOSStatus);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetBreakOnClientHello(SSLContextRef sslContext, int32_t setBreak, int32_t* pOSStatus)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SslSetSessionOption(sslContext, kSSLSessionOptionBreakOnClientHello, setBreak, pOSStatus);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetCertificate(SSLContextRef sslContext, CFArrayRef certRefs)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // The underlying call handles NULL inputs, so just pass it through
    return SSLSetCertificate(sslContext, certRefs);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslSetTargetName(SSLContextRef sslContext,
                                           const char* pszTargetName,
                                           int32_t cbTargetName,
                                           int32_t* pOSStatus)
{
    if (pOSStatus != NULL)
        *pOSStatus = noErr;

    if (sslContext == NULL || pszTargetName == NULL || pOSStatus == NULL)
        return -1;

    if (cbTargetName < 0)
        return -2;

    size_t currentLength;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    *pOSStatus = SSLGetPeerDomainNameLength(sslContext, &currentLength);

    // We'll end up walking down the path that sets the hostname more than once during
    // the handshake dance. But once the handshake starts Secure Transport isn't willing to
    // listen to this.  So, if we've already set it, don't set it again.
    if (*pOSStatus == noErr && currentLength == 0)
    {
        *pOSStatus = SSLSetPeerDomainName(sslContext, pszTargetName, (size_t)cbTargetName);
    }
#pragma clang diagnostic pop

    return *pOSStatus == noErr;
}

int32_t AppleCryptoNative_SSLSetALPNProtocols(SSLContextRef sslContext,
                                                        CFArrayRef protocols,
                                                        int32_t* pOSStatus)
{
    if (sslContext == NULL || protocols == NULL || pOSStatus == NULL)
        return -1;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // The underlying call handles NULL inputs, so just pass it through
    *pOSStatus = SSLSetALPNProtocols(sslContext, protocols);
#pragma clang diagnostic pop

    return *pOSStatus == noErr;
}

int32_t AppleCryptoNative_SSLSetALPNProtocol(SSLContextRef sslContext, void* protocol, int length, int32_t* pOSStatus)
{
    if (sslContext == NULL || protocol == NULL || length <= 0 || pOSStatus == NULL)
        return -1;

    CFStringRef value = CFStringCreateWithBytes(NULL, protocol, length, kCFStringEncodingASCII, 0);
    if (!value)
    {
        *pOSStatus = errSecMemoryError;
        return -2;
    }

    CFArrayRef protocolList = CFArrayCreate(kCFAllocatorDefault, (const void **)&value, 1, &kCFTypeArrayCallBacks);
    if (!protocolList)
    {
        CFRelease(value);
        *pOSStatus = errSecMemoryError;
        return -2;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    *pOSStatus = SSLSetALPNProtocols(sslContext, protocolList);
#pragma clang diagnostic pop

    if  (*pOSStatus == 0)
    {
        struct SSLContext* ctx = (struct SSLContext*)sslContext;
        tls_handshake_t tls = ctx->hdsk;

        // This is extra consistency check to verify that the ALPN data appeared where we expect them
        // before dereferencing sslContext
        if (tls != NULL && tls->alpnOwnData.length == (size_t)length + 1)
        {
            tls->alpn_announced = 1;
            tls->alpn_received = 1 ;
        }
        else
        {
            *pOSStatus = errSecNotAvailable;
        }
    }

    CFRelease(value);
    CFRelease(protocolList);
    return 1;
}

int32_t AppleCryptoNative_SslGetAlpnSelected(SSLContextRef sslContext, CFDataRef* protocol)
{
    if (sslContext == NULL || protocol == NULL)
        return -1;

    *protocol = NULL;
    CFArrayRef protocols = NULL;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    OSStatus osStatus = SSLCopyALPNProtocols(sslContext, &protocols);
#pragma clang diagnostic pop

    if (osStatus == noErr && protocols != NULL && CFArrayGetCount(protocols) > 0)
    {
        *protocol =
            CFStringCreateExternalRepresentation(NULL, CFArrayGetValueAtIndex(protocols, 0), kCFStringEncodingASCII, 0);
    }

    if (protocols)
        CFRelease(protocols);

    return *protocol != NULL;
}

int32_t AppleCryptoNative_SslSetIoCallbacks(SSLContextRef sslContext, SSLReadFunc readFunc, SSLWriteFunc writeFunc)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SSLSetIOFuncs(sslContext, readFunc, writeFunc);
#pragma clang diagnostic pop
}

PAL_TlsHandshakeState AppleCryptoNative_SslHandshake(SSLContextRef sslContext)
{
    if (sslContext == NULL)
        return PAL_TlsHandshakeState_Unknown;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    OSStatus osStatus = SSLHandshake(sslContext);
#pragma clang diagnostic pop

    switch (osStatus)
    {
        case noErr:
            return PAL_TlsHandshakeState_Complete;
        case errSSLWouldBlock:
            return PAL_TlsHandshakeState_WouldBlock;
        case errSSLServerAuthCompleted:
            return PAL_TlsHandshakeState_ServerAuthCompleted;
        case errSSLClientCertRequested:
            return PAL_TlsHandshakeState_ClientCertRequested;
        case errSSLClientHelloReceived:
           return PAL_TlsHandshakeState_ClientHelloReceived;
        default:
            return osStatus;
    }
}

static PAL_TlsIo OSStatusToPAL_TlsIo(OSStatus status)
{
    switch (status)
    {
        case noErr:
            return PAL_TlsIo_Success;
        case errSSLWouldBlock:
            return PAL_TlsIo_WouldBlock;
        case errSSLClosedGraceful:
            return PAL_TlsIo_ClosedGracefully;
        default:
            return status;
    }
}

PAL_TlsIo
AppleCryptoNative_SslWrite(SSLContextRef sslContext, const uint8_t* buf, uint32_t bufLen, uint32_t* bytesWritten)
{
    if (bytesWritten == NULL)
        return PAL_TlsIo_Unknown;

    size_t expected = (size_t)bufLen;
    size_t totalWritten;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    OSStatus status = SSLWrite(sslContext, buf, expected, &totalWritten);
#pragma clang diagnostic pop

    if (status != noErr)
    {
        *bytesWritten = (uint32_t)totalWritten;
        return OSStatusToPAL_TlsIo(status);
    }

    return PAL_TlsIo_Success;
}

PAL_TlsIo AppleCryptoNative_SslRead(SSLContextRef sslContext, uint8_t* buf, uint32_t bufLen, uint32_t* written)
{
    if (written == NULL)
        return PAL_TlsIo_Unknown;

    size_t writtenSize = 0;
    size_t bufSize = (size_t)bufLen;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    OSStatus status = SSLRead(sslContext, buf, bufSize, &writtenSize);
#pragma clang diagnostic pop

    if (writtenSize > UINT_MAX)
    {
        // This shouldn't happen, because we passed a uint32_t as the initial buffer size.
        // But, just in case it does, report back that we're no longer in a known state.
        return PAL_TlsIo_Unknown;
    }

    *written = (uint32_t)writtenSize;

    if (writtenSize == 0 && status == errSSLWouldBlock)
    {
        SSLSessionState state;
        memset(&state, 0, sizeof(SSLSessionState));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        OSStatus localStatus = SSLGetSessionState(sslContext, &state);

        if (localStatus == noErr && state == kSSLHandshake)
        {
            return PAL_TlsIo_Renegotiate;
        }
#pragma clang diagnostic pop
    }

    return OSStatusToPAL_TlsIo(status);
}

int32_t AppleCryptoNative_SslShutdown(SSLContextRef sslContext)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return SSLClose(sslContext);
#pragma clang diagnostic pop
}

int32_t AppleCryptoNative_SslGetProtocolVersion(SSLContextRef sslContext, PAL_SslProtocol* pProtocol)
{
    if (pProtocol != NULL)
        *pProtocol = 0;

    if (sslContext == NULL || pProtocol == NULL)
        return errSecParam;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SSLProtocol protocol = kSSLProtocolUnknown;
    OSStatus osStatus = SSLGetNegotiatedProtocolVersion(sslContext, &protocol);

    if (osStatus == noErr)
    {
        PAL_SslProtocol matchedProtocol = PAL_SslProtocol_None;

        if (protocol == kTLSProtocol13)
            matchedProtocol = PAL_SslProtocol_Tls13;
        else if (protocol == kTLSProtocol12)
            matchedProtocol = PAL_SslProtocol_Tls12;
        else if (protocol == kTLSProtocol11)
            matchedProtocol = PAL_SslProtocol_Tls11;
        else if (protocol == kTLSProtocol1)
            matchedProtocol = PAL_SslProtocol_Tls10;
        else if (protocol == kSSLProtocol3)
            matchedProtocol = PAL_SslProtocol_Ssl3;
        else if (protocol == kSSLProtocol2)
            matchedProtocol = PAL_SslProtocol_Ssl2;

        *pProtocol = matchedProtocol;
    }
#pragma clang diagnostic pop

    return osStatus;
}

int32_t AppleCryptoNative_SslGetCipherSuite(SSLContextRef sslContext, uint16_t* pCipherSuiteOut)
{
    if (pCipherSuiteOut == NULL)
    {
        return errSecParam;
    }

    SSLCipherSuite cipherSuite;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    OSStatus status = SSLGetNegotiatedCipher(sslContext, &cipherSuite);
#pragma clang diagnostic pop
    *pCipherSuiteOut = (uint16_t)cipherSuite;

    return status;
}

int32_t AppleCryptoNative_SslSetEnabledCipherSuites(SSLContextRef sslContext, const uint32_t* cipherSuites, int32_t numCipherSuites)
{
    // Max numCipherSuites is 2^16 (all possible cipher suites)
    assert(numCipherSuites < (1 << 16));

#if !defined(TARGET_ARM64) && !defined(TARGET_IOS) && !defined(TARGET_TVOS)
    if (sizeof(SSLCipherSuite) == sizeof(uint32_t))
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // macOS & MacCatalyst x64
        return SSLSetEnabledCiphers(sslContext, (const SSLCipherSuite *)cipherSuites, (size_t)numCipherSuites);
#pragma clang diagnostic pop
    }
    else
#endif
    {
        // MacCatalyst arm64, iOS, tvOS, watchOS
        SSLCipherSuite* cipherSuites16 = (SSLCipherSuite*)calloc((size_t)numCipherSuites, sizeof(SSLCipherSuite));

        if (cipherSuites16 == NULL)
        {
            return errSSLInternal;
        }

        for (int i = 0; i < numCipherSuites; i++)
        {
            cipherSuites16[i] = (SSLCipherSuite)cipherSuites[i];
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        OSStatus status = SSLSetEnabledCiphers(sslContext, cipherSuites16, (size_t)numCipherSuites);
#pragma clang diagnostic pop

        free(cipherSuites16);
        return status;
    }
}

PALEXPORT int32_t AppleCryptoNative_SslSetCertificateAuthorities(SSLContextRef sslContext, CFArrayRef certificates, int32_t replaceExisting)
{
#if defined(TARGET_OSX)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // The underlying call handles NULL inputs, so just pass it through
    return SSLSetCertificateAuthorities(sslContext, certificates, replaceExisting > 0 ? 1 : 0);
#pragma clang diagnostic pop

#else
    // ignore unused parameters
    (void)sslContext;
    (void)certificates;
    (void)replaceExisting;
    return 0;
#endif
}
