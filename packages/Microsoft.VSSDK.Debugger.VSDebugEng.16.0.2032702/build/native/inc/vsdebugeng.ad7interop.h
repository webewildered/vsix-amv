/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    vsdebugeng.ad7interop.h

Abstract:

    This file includes interfaces for accessing the Concord (Dkm) API from Visual Studio
    packages or addins.

--*/

#pragma once

#ifndef __cplusplus
    #error This file requires C++ compilation (use a .cpp suffix)
#endif

#if defined(_MANAGED) || defined(__cplusplus_cli)
    #error This file should not be included in code copiled with /clr. Reference the managed assembly instead.
#endif

namespace Microsoft {
namespace VisualStudio {
namespace Debugger {

namespace CallStack {
    // This interface is consumed by Visual Studio packages or addins which wish to 
    // access the Concord API to obtain more detailed information about the debugged
    // process. It is implemented by stack frame objects from the debugger automation 
    // layer (EnvDTE), by the SDM layer, and by the Concord debug engine. This interface
    // may only be used from the main thread of Visual Studio.
    interface DECLSPEC_NOVTABLE DECLSPEC_UUID("a445f261-5a52-4d1a-82e1-e56453ebf8a6") IVsWrappedDkmStackFrame : public IUnknown
    {
        // Obtains the DkmStackFrame object which backs this higher-level stack frame 
        // object. If the stack frame object is not backed by a DkmStackFrame (ex: frame
        // from the SQL debug engine), the implementation we return S_FALSE.
        // ppDkmFrame     : [Out,Optional] backing DkmStackFrame
        virtual HRESULT STDMETHODCALLTYPE GetDkmFrameObject(
            _Deref_out_opt_ class DkmStackFrame** ppDkmFrame
            ) = 0;
    };
};

namespace DefaultPort {
    // This interface is implemented by SDM default port objects (IDebugPort2), and transport connection
    // objects (IVsDebuggerDeployConnection). It provides direct access to the underlying DkmTransportConnection. 
    interface DECLSPEC_NOVTABLE DECLSPEC_UUID("96e1374b-33e5-4128-9f22-499d00e94aba") IVsWrappedDkmTransportConnection : public IUnknown
    {
        // Obtains the DkmTransportConnection object which backs this object. This can be
        // used to bridge from the AD7 or debugger deploy API, to the debugger engine (Dkm)
        // APIs. As an example, this can be used to send DkmCustomMessages.
        //
        // When extracting  from a port, this will fail in remote debugging scenarios if the 
        // port is not currently connected, and reconnect was unsuccessful.
        //
        // When extracting from a deployment connection, the caller must still hold onto 
        // deployConnection to avoid the underlying DkmTransportConnection from being disposed.
        // Note that, by default, concord components will be unloaded during stop debugging.
        // This behavior can be overwritten by setting 'StayLoadedForDeployConnection="true"'
        // in the component's .vsdconfigxml file. This is useful if the caller wants to
        // extract the deploy connection in order to send custom messages and wants to do so
        // after the debugger session ended.
        //
        // ppConnection     : [Out] backing DkmTransportConnection
        virtual HRESULT STDMETHODCALLTYPE GetDkmTransportConnection(
            _Deref_out_opt_ class DkmTransportConnection** ppConnection
            ) = 0;
    };
};

namespace Script {
    // This interface is implemented by IDebugProperty2 objects that are backed by script documents and
    // is used provide access to the DkmScriptDocument.
    interface DECLSPEC_NOVTABLE DECLSPEC_UUID("93852B63-17C6-4CBF-A552-F2396489B562") IVsWrappedDkmScriptDocument : public IUnknown
    {
        // Gets the underlying DkmScriptDocument.  If there is no backing DkmScriptDocument,
        // this method will return S_FALSE.
        virtual HRESULT STDMETHODCALLTYPE GetDkmScriptDocument(
            _Deref_out_opt_ class DkmScriptDocument** ppScriptDocument
            ) = 0;
    };
};

namespace Evaluation {
    // This interface is implemented by AD7 IDebugProperty3 objects, and provides access to 
    // the underlying DkmSuccessEvaluationResult. 
    interface DECLSPEC_NOVTABLE DECLSPEC_UUID("D80639B5-B128-4E99-B089-9E6314DB7399") IVsWrappedDkmSuccessEvaluationResult : public IUnknown
    {
        // Gets the underlying DkmSuccessEvaluationResult.
        // If there is no backing DkmSuccessEvaluationResult, this method will return S_FALSE.
        virtual HRESULT STDMETHODCALLTYPE GetDkmSuccessEvaluationResult(
        _Deref_out_opt_ class DkmSuccessEvaluationResult** ppDkmSuccessEvaluationResult
        ) = 0;
    };
};

namespace Symbols {
    /// This interface is implemented by AD7 IDebugDocumentContext2 objects. It provides access to the underlying DkmInstructionSymbol.
    interface DECLSPEC_NOVTABLE DECLSPEC_UUID("8E122A39-3F5C-4B61-9777-E12C35310BB0") IVsWrappedDkmInstructionSymbol : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE GetDkmInstructionSymbol(
            _Deref_out_ class DkmInstructionSymbol** ppInstructionSymbol
        ) = 0;
    };
};
};
};
};
