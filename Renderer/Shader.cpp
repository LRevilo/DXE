#include "pch.h"
#include "Shader.h"




namespace DXE
{

    Shader::Shader() {
    }
    Shader::Shader(const std::wstring& path) : m_Path(path) {}
    Shader::Shader(const std::string& name, const std::wstring& path)
        : m_Name(name), m_Path(path) {}
    Shader::Shader(const std::string& name, const std::string& source) 
        : m_Name(name), m_Source(source) {}
    Shader::Shader(const std::string& name, const ShaderStruct& shaderStruct) {
        LoadFromBytecode(name, shaderStruct);
    }


    void Shader::LoadFromBytecode(const std::string& name, const ShaderStruct& shaderStruct)
    {
        auto device = Renderer::Device(); // your function to get ID3D11Device*

        m_Name = name;
        
        if (shaderStruct.vs)
        {
            HRESULT hr = device->CreateVertexShader(
                shaderStruct.vs->data,
                shaderStruct.vs->length,
                nullptr,
                &m_VertexShader
            );
            hasVertex = SUCCEEDED(hr);

            // Optionally reflect input layout if vertex shader exists
            if (hasVertex)
            {

                // Reflect to auto-generate Input Layout
                ID3D11ShaderReflection* rawReflection = nullptr;
                HRESULT hr = D3DReflect(
                    shaderStruct.vs->data,
                    shaderStruct.vs->length,
                    __uuidof(ID3D11ShaderReflection),
                    (void**)&rawReflection
                );
                assert(SUCCEEDED(hr));

                ReflectInputLayout(rawReflection); // call your existing reflection function

                hr = Renderer::Device()->CreateInputLayout(
                    m_InputLayoutDesc.data(),
                    static_cast<uint32_t>(m_InputLayoutDesc.size()),
                    shaderStruct.vs->data,
                    shaderStruct.vs->length,
                    m_InputLayout.GetAddressOf()
                );
                assert(SUCCEEDED(hr));

                if (rawReflection) rawReflection->Release();
            }
        }

        if (shaderStruct.ps)
        {
            HRESULT hr = device->CreatePixelShader(
                shaderStruct.ps->data,
                shaderStruct.ps->length,
                nullptr,
                &m_PixelShader
            );
            hasPixel = SUCCEEDED(hr);
        }

        if (shaderStruct.cs)
        {
            HRESULT hr = device->CreateComputeShader(
                shaderStruct.cs->data,
                shaderStruct.cs->length,
                nullptr,
                &m_ComputeShader
            );
            hasCompute = SUCCEEDED(hr);
        }

        if (shaderStruct.hs)
        {
            HRESULT hr = device->CreateHullShader(
                shaderStruct.hs->data,
                shaderStruct.hs->length,
                nullptr,
                &m_HullShader
            );
            hasTessellation = SUCCEEDED(hr);
        }

        if (shaderStruct.ds)
        {
            HRESULT hr = device->CreateDomainShader(
                shaderStruct.ds->data,
                shaderStruct.ds->length,
                nullptr,
                &m_DomainShader
            );
            hasTessellation = SUCCEEDED(hasTessellation && SUCCEEDED(hr));
        }

        if (shaderStruct.gs)
        {
            HRESULT hr = device->CreateGeometryShader(
                shaderStruct.gs->data,
                shaderStruct.gs->length,
                nullptr,
                &m_GeometryShader
            );
            hasGeometry = SUCCEEDED(hr);
        }


    }
    void Shader::CompileFromSource(std::unordered_map <std::string, std::string> includesShaderMap) {

        //std::cout << "Compiling: " << m_Name << "\n";
        DXE_INFO("Compiling: ", m_Name);
        //std::cout << "Source: \n" << m_Source << "\n";
        ID3DBlob* compileErrors = nullptr;;


        ShaderIncludeHandler includeHandler(includesShaderMap);

        // === Vertex Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        HRESULT hResult = D3DCompile(m_Source.c_str(),m_Source.size(),m_Name.c_str(), nullptr, &includeHandler, "vs_main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), &compileErrors);
        if (SUCCEEDED(hResult)) {
            hResult = Renderer::Device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_VertexShader.GetAddressOf());
            assert(SUCCEEDED(hResult));
            hasVertex = true;


            // Reflect to auto-generate Input Layout
            ID3D11ShaderReflection* rawReflection = nullptr;
            HRESULT hr;
            hr = D3DReflect(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&rawReflection);
            ReflectInputLayout(rawReflection);

            hResult = Renderer::Device()->CreateInputLayout(m_InputLayoutDesc.data(), static_cast<uint32_t>(m_InputLayoutDesc.size()), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_InputLayout.GetAddressOf());
            assert(SUCCEEDED(hResult));
        }
        else {
            DXE_WARN("Vertex shader not compiled (maybe not used in this shader).");
        }


        // === Hull Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> hsBlob;
        hResult = D3DCompile(m_Source.c_str(), m_Source.size(), m_Name.c_str(), nullptr, &includeHandler,
            "hs_main", "hs_5_0", 0, 0, hsBlob.GetAddressOf(), &compileErrors);

        if (SUCCEEDED(hResult)) {
            hResult = Renderer::Device()->CreateHullShader(hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), nullptr, m_HullShader.GetAddressOf());
            assert(SUCCEEDED(hResult));
            hasTessellation = true;
        }
        else {
            DXE_WARN("Hull shader not compiled (maybe not used in this shader).");
        }

        // === Domain Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> dsBlob;
        hResult = D3DCompile(m_Source.c_str(), m_Source.size(), m_Name.c_str(), nullptr, &includeHandler,  "ds_main", "ds_5_0", 0, 0, dsBlob.GetAddressOf(), &compileErrors);

        if (SUCCEEDED(hResult)) {
            hResult = Renderer::Device()->CreateDomainShader(dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), nullptr, m_DomainShader.GetAddressOf());
            assert(SUCCEEDED(hResult));
            hasTessellation = true;
        }
        else {
            DXE_WARN("Domain shader not compiled (maybe not used in this shader).");
        }

        // === Geometry Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> gsBlob;
        hResult = D3DCompile( m_Source.c_str(), m_Source.size(), m_Name.c_str(), nullptr, &includeHandler, "gs_main", "gs_5_0", 0, 0, gsBlob.GetAddressOf(), &compileErrors);
        if (SUCCEEDED(hResult)) {
            hResult = Renderer::Device()->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, m_GeometryShader.GetAddressOf());
            assert(SUCCEEDED(hResult));
            hasGeometry = true;
        }
        else {
            DXE_WARN("Geometry shader not compiled (maybe not used in this shader).");
        }



         // === Compute Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> csBlob;
        hResult = D3DCompile(m_Source.c_str(), m_Source.size(), m_Name.c_str(), nullptr, &includeHandler,
            "cs_main", "cs_5_0", 0, 0, csBlob.GetAddressOf(), &compileErrors);

        if (FAILED(hResult))
        {
            if (compileErrors)
            {
                // Log the errors
                DXE_ERROR("Compute shader compile errors:\n", (char*)compileErrors->GetBufferPointer());
                compileErrors->Release();
            }
            else
            {
                DXE_ERROR("Compute shader failed to compile for unknown reasons.");
            }
        }


        if (SUCCEEDED(hResult)) {
        hResult = DXE::Renderer::Device()->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, m_ComputeShader.GetAddressOf());
            if(SUCCEEDED(hResult)) {
                hasCompute = true;
            }
            else {
                DXE_ERROR("Compute shader not created (WHY?).");
            }
        }
        else {
            DXE_WARN("Compute shader not compiled (maybe not used in this shader).");
        }
        //


        // === Pixel Shader ===
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        hResult = D3DCompile(m_Source.c_str(), m_Source.size(), m_Name.c_str(), nullptr, &includeHandler, "ps_main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), &compileErrors);
        if (SUCCEEDED(hResult)) {
            hResult = Renderer::Device()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_PixelShader.GetAddressOf());
            assert(SUCCEEDED(hResult));
            hasPixel = true;
        }
        else {
            DXE_WARN("Pixel shader not compiled (maybe not used in this shader).");
        }
        //Renderer::CheckCompileErrors(hResult, compileErrors);



        DXE_INFO("Compiling Done: ", m_Name);
    }
    void Shader::Bind() {



        Renderer::Context()->IASetInputLayout(m_InputLayout.Get());
        Renderer::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (hasVertex) {  Renderer::Context()->VSSetShader(m_VertexShader.Get(), nullptr, 0);  }
   
        if (hasTessellation) {
            Renderer::Context()->HSSetShader(m_HullShader.Get(), nullptr, 0);
            Renderer::Context()->DSSetShader(m_DomainShader.Get(), nullptr, 0);
            Renderer::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
        }
        else {
            Renderer::Context()->HSSetShader(nullptr, nullptr, 0);
            Renderer::Context()->DSSetShader(nullptr, nullptr, 0);
            //Renderer::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }

        if (hasGeometry) {
            Renderer::Context()->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
            Renderer::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        }
        else {
            Renderer::Context()->GSSetShader(nullptr, nullptr, 0);
            //Renderer::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }


        //if (hasCompute) { Renderer::Context()->CSSetShader(m_ComputeShader.Get(), nullptr, 0); }
        //else { Renderer::Context()->CSSetShader(nullptr, nullptr, 0); }
 
        if (hasPixel) { Renderer::Context()->PSSetShader(m_PixelShader.Get(), nullptr, 0); }

    }

    void Shader::ReflectInputLayout(ID3D11ShaderReflection* rawReflection) {
        m_InputLayoutDesc.clear();

        
        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
        reflection = rawReflection;


        D3D11_SHADER_DESC shaderDesc;
        HRESULT hr = reflection->GetDesc(&shaderDesc);
        assert(SUCCEEDED(hr));


        // Iterate through all the input parameters
        for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = reflection->GetInputParameterDesc(i, &paramDesc);
            assert(SUCCEEDED(hr));

           
            if (paramDesc.SemanticName == nullptr || strlen(paramDesc.SemanticName) == 0) {
                //std::cerr << "ERROR: SemanticName is invalid or empty at index " << i << std::endl;
                DXE_ERROR("SemanticName is invalid or empty at index ", i);
            }
     
            // Skip over some params that arent needed
            if (strcmp(paramDesc.SemanticName, "SV_InstanceID") == 0) { continue; }

            D3D11_INPUT_ELEMENT_DESC elementDesc = {};
            elementDesc.SemanticName = paramDesc.SemanticName;
            elementDesc.SemanticIndex = paramDesc.SemanticIndex;

            if (strcmp(elementDesc.SemanticName, "INSTANCE_TRANSFORM") == 0 || strcmp(elementDesc.SemanticName, "INSTANCE_COLOR") == 0) {
                elementDesc.InputSlot = 1;
                elementDesc.InstanceDataStepRate = 1;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            }
            else {
                elementDesc.InputSlot = 0;
                elementDesc.InstanceDataStepRate = 0;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            }
            elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

            if (paramDesc.Mask == 1) {
                elementDesc.Format = DXGI_FORMAT_R32_FLOAT;  //float
            }
            else if (paramDesc.Mask == 3) {
                elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;  //float2
            }
            else if (paramDesc.Mask == 7) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;  //float3
            }
            else if (paramDesc.Mask == 15) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float4
            }

            // Print out details about the input element
            if (false) {
                std::cout << "  Input Parameter " << i << ":\n";
                std::cout << "  Semantic Name: " << elementDesc.SemanticName << "\n";
                std::cout << "  Semantic Index: " << paramDesc.SemanticIndex << "\n";
                std::cout << "  Format: " << elementDesc.Format << "\n";
                std::cout << "  Input Slot: " << elementDesc.InputSlot << "\n";
                std::cout << "  Instance Data Step Rate: " << elementDesc.InstanceDataStepRate << "\n";
                std::cout << "  Input Slot Class: " << (elementDesc.InputSlotClass == D3D11_INPUT_PER_VERTEX_DATA ? "PER_VERTEX_DATA" : "PER_INSTANCE_DATA") << "\n";
                std::cout << "  Aligned Byte Offset: " << elementDesc.AlignedByteOffset << "\n";
            }

            m_InputLayoutDesc.push_back(elementDesc);
        }


        for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i) {
            ID3D11ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
            if (!constantBuffer) {
                DXE_WARN("Failed to get Constant Buffer");
                continue;
            }

            D3D11_SHADER_BUFFER_DESC bufferDesc;
            HRESULT hr = constantBuffer->GetDesc(&bufferDesc);
            if (FAILED(hr)) {
                DXE_WARN("Failed to get Buffer Description");
                continue;
            }

            ShaderConstantBufferInfo bufferInfo;
            bufferInfo.Name = bufferDesc.Name;
            bufferInfo.Size = bufferDesc.Size;
            bufferInfo.Type = bufferDesc.Type;
            bufferInfo.VariableCount = bufferDesc.Variables;

            // Iterate over variables inside the constant buffer
            for (UINT j = 0; j < bufferDesc.Variables; ++j) {
                ID3D11ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(j);
                if (!variable) {
                    DXE_WARN("Failed to get Buffer Variable");
                    continue;
                }

                D3D11_SHADER_VARIABLE_DESC varDesc;
                hr = variable->GetDesc(&varDesc);
                if (FAILED(hr)) {
                    DXE_WARN("Failed to get Variable Description");
                    continue;
                }

                ShaderVariableInfo varInfo;
                varInfo.Name = varDesc.Name;
                varInfo.StartOffset = varDesc.StartOffset;
                varInfo.Size = varDesc.Size;
                varInfo.uFlags = varDesc.uFlags;

                bufferInfo.Variables.push_back(varInfo);
            }

            // Store in the map
            m_ConstantBufferInfo[bufferInfo.Name] = bufferInfo;
        }
    }


    void Shader::SetTexture(ID3D11ShaderResourceView* texture) {
        assert(texture != nullptr);
        Renderer::Context()->PSSetShaderResources(0, 1, &texture);
    }
}