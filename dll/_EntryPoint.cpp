// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// _EntryPoint.cpp : Implementation of CArrayMemberVisualizer

#include "stdafx.h"
#include "_EntryPoint.h"


struct ArrayMemberDesc
{
    const UINT64 base;
    const UINT64 member;
    int count;
    ArrayMemberDesc() : base(0), member(0) {}
};

struct __declspec(uuid("b45173ac-0af8-4b5c-a353-184af2754ef6"))
ArrayMemberDataItem : public IUnknown
{
public:
    const char* base;
    const char* member;
    int count;
    ULONG refs;

    ArrayMemberDataItem()
    {
        refs = 1;
    }
    
    // TODO.ma
    ULONG STDMETHODCALLTYPE AddRef() { return ++refs; }
    ULONG STDMETHODCALLTYPE Release() { return --refs; }
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void** ppvObject
    )
    {
        if (riid == IID_IUnknown)
        {
            *ppvObject = (this);
            return S_OK;
        }

        return E_NOTIMPL;
    }
};



/*

object_visualizer : winrt::implements<object_visualizer, ::IUnknown>
*/

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::EvaluateVisualizedExpression(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
    )
{
    HRESULT hr;

    // This method is called to visualize a FILETIME variable. Its basic job is to create
    // a DkmEvaluationResult object. A DkmEvaluationResult is the data that backs a row in the
    // watch window -- a name, value, and type, a flag indicating if the item can be expanded, and
    // lots of other additional properties.

    Evaluation::DkmPointerValueHome* pPointerValueHome = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
    if (pPointerValueHome == nullptr)
    {
        // This sample only handles visualizing in-memory FILETIME structures
        return E_NOTIMPL;
    }

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    // Read the FILETIME value from the target process
    DkmProcess* pTargetProcess = pVisualizedExpression->RuntimeInstance()->Process();
    ArrayMemberDesc value;
    hr = pTargetProcess->ReadMemory(pPointerValueHome->Address(), DkmReadMemoryFlags::None, &value, sizeof(value), nullptr);
    if (FAILED(hr))
    {
        // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
        return E_NOTIMPL;
    }

    DkmArray<BYTE> base, member;
    hr = pTargetProcess->ReadMemoryString(value.base, DkmReadMemoryFlags::None, 1, 0x10000, &base);
    if (FAILED(hr))
    {
        // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
        return E_NOTIMPL;
    }
    hr = pTargetProcess->ReadMemoryString(value.member, DkmReadMemoryFlags::None, 1, 0x10000, &member);
    if (FAILED(hr))
    {
        // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
        return E_NOTIMPL;
    }

    CString strValue;
    strValue = "...";
    CComPtr<DkmString> pValue;
    hr = DkmString::Create(DkmSourceString(strValue), &pValue);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmDataAddress> pAddress;
    hr = DkmDataAddress::Create(pVisualizedExpression->RuntimeInstance(), pPointerValueHome->Address(), nullptr, &pAddress);
    if (FAILED(hr))
    {
        return hr;
    }

    DkmEvaluationResultFlags_t resultFlags = DkmEvaluationResultFlags::Expandable | DkmEvaluationResultFlags::ReadOnly;

    ArrayMemberDataItem* dataItem = new ArrayMemberDataItem();
    dataItem->base = (char*)base.Members;
    dataItem->member = (char*)member.Members;
    dataItem->count = value.count;

    pVisualizedExpression->SetDataItem(DkmDataCreationDisposition_t::CreateNew, dataItem);

    CComPtr<DkmSuccessEvaluationResult> pSuccessEvaluationResult;
    hr = DkmSuccessEvaluationResult::Create(
        pVisualizedExpression->InspectionContext(),
        pVisualizedExpression->StackFrame(),
        pRootVisualizedExpression->Name(),
        pRootVisualizedExpression->FullName(),
        resultFlags,
        pValue,
        nullptr, //pEditableValue
        pRootVisualizedExpression->Type(),
        DkmEvaluationResultCategory::Class,
        DkmEvaluationResultAccessType::None,
        DkmEvaluationResultStorageType::None,
        DkmEvaluationResultTypeModifierFlags::None,
        pAddress,
        nullptr,
        (DkmReadOnlyCollection<DkmModuleInstance*>*)nullptr,
        // This sample doesn't need to store any state associated with this evaluation result, so we
        // pass `DkmDataItem::Null()` here. A more complicated extension which had associated
        // state such as an extension which took over expansion of evaluation results would likely
        // create an instance of the extension's data item class and pass the instance here.
        // More information: https://github.com/Microsoft/ConcordExtensibilitySamples/wiki/Data-Container-API
        DkmDataItem::Null(),
        &pSuccessEvaluationResult
        );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppResultObject = pSuccessEvaluationResult.Detach();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::UseDefaultEvaluationBehavior(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Out_ bool* pUseDefaultEvaluationBehavior,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
    )
{
    *pUseDefaultEvaluationBehavior = false;
    return S_OK;
}

HRESULT getChildren(
    _In_ ArrayMemberDataItem* dataItem,
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ Evaluation::DkmInspectionContext* pInspectionContext,
    _In_ UINT32 StartIndex,
    _In_ UINT32 Count,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
)
{
    HRESULT hr;

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    hr = DkmAllocArray(Count, pItems);
    if (FAILED(hr))
    {
        return hr;
    }

    for (UINT32 i = 0; i < Count; i++)
    {
        DkmInspectionContext* pParentInspectionContext = pVisualizedExpression->InspectionContext();

        int itemIndex = i + StartIndex;
        CString childExpr; childExpr.Format(L"%S[%i]%S", dataItem->base, itemIndex, dataItem->member);
        CComPtr<DkmString> childExpression;
        hr = DkmString::Create(DkmSourceString(childExpr), &childExpression);
        if (FAILED(hr))
        {
            return hr;
        }

        CAutoDkmClosePtr<DkmLanguageExpression> pLanguageExpression;
        hr = DkmLanguageExpression::Create(
            pParentInspectionContext->Language(),
            DkmEvaluationFlags::TreatAsExpression,
            childExpression,
            DkmDataItem::Null(),
            &pLanguageExpression
        );
        if (FAILED(hr))
        {
            return hr;
        }

        CComPtr<DkmEvaluationResult> pEEEvaluationResult;
        hr = pVisualizedExpression->EvaluateExpressionCallback(
            pInspectionContext,
            pLanguageExpression,
            pVisualizedExpression->StackFrame(),
            &pEEEvaluationResult
        );
        if (FAILED(hr))
        {
            return hr;
        }

        CComPtr<DkmChildVisualizedExpression> pChildVisualizedExpresion;
        hr = DkmChildVisualizedExpression::Create(
            pInspectionContext,
            pRootVisualizedExpression->VisualizerId(),
            pRootVisualizedExpression->SourceId(),
            pRootVisualizedExpression->StackFrame(),
            pRootVisualizedExpression->ValueHome(),
            pEEEvaluationResult,
            pRootVisualizedExpression,
            itemIndex,
            DkmDataItem::Null(),
            &pChildVisualizedExpresion);
        if (FAILED(hr))
        {
            return hr;
        }

        pItems->Members[i] = pChildVisualizedExpresion.Detach();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::GetChildren(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ UINT32 InitialRequestSize,
    _In_ Evaluation::DkmInspectionContext* pInspectionContext,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren,
    _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** ppEnumContext
    )
{
    HRESULT hr;

    ArrayMemberDataItem* dataItem;
    hr = pVisualizedExpression->GetDataItem<ArrayMemberDataItem>(&dataItem);
    if (FAILED(hr))
    {
        return hr;
    }

    DkmEvaluationResultEnumContext* pEnumContext;
    hr = DkmEvaluationResultEnumContext::Create(
        dataItem->count,
        pVisualizedExpression->StackFrame(),
        pInspectionContext,
        dataItem,
        &pEnumContext);
    if (FAILED(hr))
    {
        return hr;
    }
    *ppEnumContext = pEnumContext;

    return getChildren(dataItem, pVisualizedExpression, pInspectionContext, 0, dataItem->count, pInitialChildren);
}

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::GetItems(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ Evaluation::DkmEvaluationResultEnumContext* pEnumContext,
    _In_ UINT32 StartIndex,
    _In_ UINT32 Count,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
    )
{
    HRESULT hr;

    ArrayMemberDataItem* dataItem;
    hr = pVisualizedExpression->GetDataItem<ArrayMemberDataItem>(&dataItem);
    if (FAILED(hr))
    {
        return hr;
    }

    return getChildren(dataItem, pVisualizedExpression, pEnumContext->InspectionContext(), 0, dataItem->count, pItems);
}

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::SetValueAsString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ DkmString* pValue,
    _In_ UINT32 Timeout,
    _Deref_out_opt_ DkmString** ppErrorText
    )
{
    // This sample delegates setting values to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CArrayMemberVisualizer::GetUnderlyingString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ DkmString** ppStringValue
    )
{
    // FILETIME doesn't have an underlying string (no DkmEvaluationResultFlags::RawString), so this method
    // doesn't need to be implemented
    return E_NOTIMPL;
}
