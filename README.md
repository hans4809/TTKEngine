# 크래프톤 정글 테크랩 1기 자체 엔진 프로젝트👥
<img width="569" height="88" alt="image" src="https://github.com/user-attachments/assets/f60b757d-a279-4fb0-8557-93c611e97036" />

# TTKEngine
<img width="1633" height="1070" alt="image (10)" src="https://github.com/user-attachments/assets/be99a65b-f2be-45ae-a729-359b5d63b75f" />

## 시연영상 
#### ⬇ Link Here ⬇
[https://youtu.be/8ucHX0Cfe6w](https://www.youtube.com/watch?v=nwQT1LWeRIo)
 
## 기술 스택 💻
<img width="306" height="345" alt="image" src="https://github.com/user-attachments/assets/9d3942bd-9553-4c20-875d-866b81b6c4e4" />
<img width="306" height="345" alt="pngwing com (1)" src="https://github.com/user-attachments/assets/faffd92b-38e7-4d93-8943-8f4ccb1203de" />

## 구현 내용
### Property를 활용한 Binary Save/Load/Duplicate 구현
```{cpp}
void FObjectProperty::Serialize(FArchive2& Ar, void* DataPtr) const
{
    UObject*& Ref = *reinterpret_cast<UObject**>(DataPtr);
    if (Ar.IsSaving())
    {
        bool bHas = Ref != nullptr;
        Ar.SerializeRaw(&bHas, sizeof(bHas));
        if (bHas)
        {
            Serializer::Save(Ar, Ref);
        }
    }
    else
    {
        bool bHas = false;
        Ar.SerializeRaw(&bHas, sizeof(bHas));
        if (bHas)
        {
            Ref = Serializer::Load(Ar);
        }
        else
        {
            Ref = nullptr;
        }
    }
}

void FObjectProperty::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    UObject* SrcObj = *reinterpret_cast<UObject* const*>(SrcPtr);
    UObject* DstObj = SrcObj ? Duplicator.DuplicateObject(SrcObj) : nullptr;
    *reinterpret_cast<UObject**>(DstPtr) = DstObj;
}

void UObject::Serialize(FArchive2& Ar)
{
    // 이 객체가 속한 클래스(UStruct) 정보 가져오기
    UClass* ClassInfo = GetClass();
    if (!ClassInfo) return;

    for (UStruct* Struct = GetClass(); Struct; Struct = Struct->GetSuperStruct())
    {
        for (const FProperty* Prop : Struct->GetProperties())
        {
            void* DataPtr = reinterpret_cast<char*>(this) + Prop->Offset;
            Prop->Serialize(Ar, DataPtr);
        }
    }
}
```

### Shader Reflection을 활용한 Shader Pipeline 구성

```{cpp}
void FGraphicsDevice::ExtractVertexShaderInfo(ID3DBlob* shaderBlob, TArray<FConstantBufferInfo>& OutCBInfos, ID3D11InputLayout*& OutInputLayout) const
{
    ID3D11ShaderReflection* pReflector = nullptr;
    HRESULT hr = D3DReflect(shaderBlob->GetBufferPointer(),
                            shaderBlob->GetBufferSize(),
                            IID_ID3D11ShaderReflection,
                            reinterpret_cast<void**>(&pReflector));
    
    if (FAILED(hr) || !pReflector)
    {
        return;
    }

    D3D11_SHADER_DESC shaderDesc = {};
    hr = pReflector->GetDesc(&shaderDesc);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pReflector);
        return;
    }

    OutCBInfos = ExtractConstantBufferInfos(pReflector, shaderDesc);
    
    // 실패 시에도 반드시 nullptr이 넘어가도록 미리 초기화
    OutInputLayout = nullptr;
    ExtractInputLayout(shaderBlob, pReflector, shaderDesc, OutInputLayout);
    //OutInputLayout = ExtractInputLayout(shaderBlob, pReflector, shaderDesc);
    
    SAFE_RELEASE(pReflector);
}

void FGraphicsDevice::ExtractPixelShaderInfo(ID3DBlob* shaderBlob, TArray<FConstantBufferInfo>& OutCBInfos)
{
    ID3D11ShaderReflection* pReflector = nullptr;
    HRESULT hr = D3DReflect(shaderBlob->GetBufferPointer(),
                            shaderBlob->GetBufferSize(),
                            IID_ID3D11ShaderReflection,
                            reinterpret_cast<void**>(&pReflector));
    
    if (FAILED(hr) || !pReflector)
    {
        return;
    }

    D3D11_SHADER_DESC shaderDesc = {};
    hr = pReflector->GetDesc(&shaderDesc);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pReflector);
        return;
    }

    OutCBInfos = ExtractConstantBufferInfos(pReflector, shaderDesc);
}

TArray<FConstantBufferInfo> FGraphicsDevice::ExtractConstantBufferInfos(ID3D11ShaderReflection* InReflector, const D3D11_SHADER_DESC& InShaderDecs)
{
    TArray<FConstantBufferInfo> CBInfos;
    
    // 모든 상수 버퍼에 대해 이름을 추출
    for (UINT i = 0; i < InShaderDecs.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* pCB = InReflector->GetConstantBufferByIndex(i);
        if (pCB)
        {
            D3D11_SHADER_BUFFER_DESC cbDesc = {};
            const HRESULT hr = pCB->GetDesc(&cbDesc);
            if(cbDesc.Type != D3D_CT_CBUFFER)
            {
                continue;
            }
            
            const FString CBName = cbDesc.Name;
            uint32 BindingSlot = 0;
            
            for (UINT j = 0; j < InShaderDecs.BoundResources; ++j)
            {
                D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
                if (SUCCEEDED(InReflector->GetResourceBindingDesc(j, &bindDesc)))
                {
                    if (bindDesc.Type != D3D_SIT_CBUFFER)
                    {
                        continue;
                    }
                    
                    if (_stricmp(bindDesc.Name, cbDesc.Name) == 0)  // 이름 비교, 대소문자 무시
                    {
                        BindingSlot = bindDesc.BindPoint;
                        break;
                    }
                }
            }
            CBInfos.Add(FConstantBufferInfo(CBName, cbDesc.Size, BindingSlot));
        }
    }
    
    return CBInfos;
}
```
