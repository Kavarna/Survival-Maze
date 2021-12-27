#include "Application.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"

#include "imgui/imgui.h"

Application::Application() :
    mSceneLight((unsigned int)Direct3D::kBufferCount)
{
}

bool Application::OnInit(ID3D12GraphicsCommandList* initializationCmdList, ID3D12CommandAllocator* cmdAllocator)
{
    mSceneLight.SetAmbientColor(0.1f, 0.1f, 0.1f, 1.0f);
    // mSceneLight.SetAmbientColor(1.0f, 1.0f, 1.0f, 1.0f);
    mSceneLight.AddDirectionalLight("Sun", DirectX::XMFLOAT3(-0.5f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));

    CHECK(InitModels(initializationCmdList, cmdAllocator), false, "Unable to initialize all models");

    return true;
}

bool Application::OnUpdate(FrameResources* frameResources, float dt)
{
    ReactToKeyPresses(dt);
    UpdateCamera(frameResources);
    UpdateModels(frameResources);
    mSceneLight.UpdateLightsBuffer(frameResources->LightsBuffer);
    return true;
}

bool Application::OnRender(ID3D12GraphicsCommandList* cmdList, FrameResources* frameResources)
{
    auto d3d = Direct3D::Get();
    auto pipelineManager = PipelineManager::Get();
    frameResources->VertexBatchRenderer.Begin();

    FLOAT backgroundColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

    auto rootSignatureResult = pipelineManager->GetRootSignature(PipelineType::InstancedColorMaterialLight);
    CHECK(rootSignatureResult.Valid(), false, "Unable to retrieve pipeline and root signature");
    auto rootSignature = rootSignatureResult.Get();

    cmdList->RSSetViewports(1, &mViewport);
    cmdList->RSSetScissorRects(1, &mScissors);

    cmdList->SetGraphicsRootSignature(rootSignature);

    auto backbufferHandle = d3d->GetBackbufferHandle();
    auto dsvHandle = d3d->GetDSVHandle();

    cmdList->ClearRenderTargetView(backbufferHandle, backgroundColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    cmdList->OMSetRenderTargets(1, &backbufferHandle, TRUE, &dsvHandle);

    Model::Bind(cmdList);
    ResetModelsInstances();

    mMaze.Render();
    mPlayer.Render();
    mPlayer.RenderDebug(frameResources->VertexBatchRenderer);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderModels(cmdList, frameResources);


    frameResources->VertexBatchRenderer.End(cmdList);
    return true;
}


bool Application::OnRenderGUI()
{
    return true;
}

bool Application::OnResize()
{
    mViewport.Width = (FLOAT)mClientWidth;
    mViewport.Height = (FLOAT)mClientHeight;
    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;

    mScissors.left = 0;
    mScissors.top = 0;
    mScissors.right = mClientWidth;
    mScissors.bottom = mClientWidth;


    // mCamera.Create(mCamera.GetPosition() (float)mClientWidth / mClientHeight);
    mCamera.Create((float)mClientWidth / mClientHeight);
    return true;
}

std::unordered_map<uuids::uuid, uint32_t> Application::GetInstanceCount()
{
    std::unordered_map<uuids::uuid, uint32_t> result;
    for (const auto& model : mModels)
    {
        result[model.GetUUID()] = model.GetInstanceCount();
    }
    return result;
}

uint32_t Application::GetModelCount()
{
    return (uint32_t)mModels.size();
}

ID3D12PipelineState* Application::GetBeginFramePipeline()
{
    auto pipelineResult = PipelineManager::Get()->GetPipeline(PipelineType::InstancedColorMaterialLight);
    CHECK(pipelineResult.Valid(), nullptr, "Cannot get pipeline for begin frame");
    return pipelineResult.Get();
}

bool Application::InitModels(ID3D12GraphicsCommandList* initializationCmdList, ID3D12CommandAllocator* cmdAllocator)
{
    auto d3d = Direct3D::Get();
    auto materialManager = MaterialManager::Get();
    CHECK_HR(cmdAllocator->Reset(), false);
    CHECK_HR(initializationCmdList->Reset(cmdAllocator, nullptr), false);

    mModels.emplace_back(Direct3D::kBufferCount, 0);
    CHECK(mModels.back().Create("Resources\\Cube.obj"), false, "Unable to load Suzanne");
    mCubeModel = &mModels.back();

    ComPtr<ID3D12Resource> intermediaryResources[2];
    CHECK(Model::InitBuffers(initializationCmdList, intermediaryResources), false, "Unable to initialize buffers for models");

    // mCamera.Create({ 0.0f, 2.0f, -3.0f }, (float)mClientWidth / mClientHeight);
    mCamera.Create((float)mClientWidth / mClientHeight);
    CHECK(mPlayer.Create(mCubeModel), false, "Unable to create player model");
    mPlayer.mModel.Translate(0.0f, 2.0f, 0.0f);

    Maze::MazeInitializationInfo mazeInfo = {};
    mazeInfo.rows = Random::get(10, 20);
    mazeInfo.cols = Random::get(10, 20);
    mazeInfo.tileWidth = 3.0f;
    mazeInfo.tileDepth = 3.0f;
    mazeInfo.cubeModel = mCubeModel;
    auto startPositionResult = mMaze.Create(mazeInfo);
    CHECK(startPositionResult.Valid(), false, "Unable to create maze");


    CHECK_HR(initializationCmdList->Close(), false);
    d3d->Flush(initializationCmdList, mFence.Get(), ++mCurrentFrame);

    return true;
}

void Application::ReactToKeyPresses(float dt)
{
    static int lastScrollWheelValue = 0;
    auto kb = mKeyboard->GetState();
    auto mouse = mMouse->GetState();
    bool mPlayerMoved = false;

    if (kb.Escape)
    {
        PostQuitMessage(0);
    }

    if (kb.Up)
    {
        mPlayer.Walk(dt);
        mPlayerMoved = true;
    }
    if (!mPlayerMoved)
    {
        mPlayer.ResetAnimation();
    }

    if (!mMenuActive)
    {
        //if (kb.W)
        //{
        //    mCamera.MoveForward(dt);
        //}
        //if (kb.S)
        //{
        //    mCamera.MoveBackward(dt);
        //}
        //if (kb.D)
        //{
        //    mCamera.MoveRight(dt);
        //}
        //if (kb.A)
        //{
        //    mCamera.MoveLeft(dt);
        //}

        mouse.x = Math::clamp(mouse.x, -25, 25);
        mouse.y = Math::clamp(mouse.y, -25, 25);
        mCamera.Update(dt, (float)mouse.x, (float)mouse.y);
        int scrollValue = mouse.scrollWheelValue - lastScrollWheelValue;
        mCamera.AdjustZoom((float)scrollValue * 0.01f);
    }
    else
    {
        mCamera.Update(dt, 0.0f, 0.0f);
    }

    static bool bRightClick = false;
    if (mouse.rightButton && !bRightClick)
    {
        bRightClick = true;
        if (mMenuActive)
        {
            mMouse->SetMode(DirectX::Mouse::Mode::MODE_RELATIVE);
            while (ShowCursor(FALSE) > 0);
        }
        else
        {
            mMouse->SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
            while (ShowCursor(TRUE) <= 0);
        }
        mMenuActive = !mMenuActive;
    }
    else if (!mouse.rightButton)
        bRightClick = false;

    lastScrollWheelValue = mouse.scrollWheelValue;
}

void Application::UpdateCamera(FrameResources* frameResources)
{
    if (mCamera.DirtyFrames > 0)
    {
        auto mappedMemory = frameResources->PerPassBuffers.GetMappedMemory();
        mappedMemory->View = DirectX::XMMatrixTranspose(mCamera.GetView());
        mappedMemory->Projection = DirectX::XMMatrixTranspose(mCamera.GetProjection());

        mappedMemory->CameraPosition = mCamera.GetPosition();

        mCamera.DirtyFrames--;
    }
}

void Application::UpdateModels(FrameResources* frameResources)
{
}

void Application::RenderModels(ID3D12GraphicsCommandList* cmdList, FrameResources* frameResources)
{
    auto textureManager = TextureManager::Get();

    cmdList->SetGraphicsRootConstantBufferView(0, frameResources->PerPassBuffers.GetGPUVirtualAddress());
    cmdList->SetGraphicsRootConstantBufferView(2, frameResources->LightsBuffer.GetGPUVirtualAddress());

    cmdList->SetDescriptorHeaps(1, textureManager->GetSrvUavDescriptorHeap().GetAddressOf());

    for (unsigned int i = 0; i < mModels.size(); ++i)
    {
        auto materialBufferAddress = frameResources->MaterialsBuffers.GetGPUVirtualAddress();
        const auto* objectMaterial = mModels[i].GetMaterial();
        materialBufferAddress += (uint64_t)objectMaterial->ConstantBufferIndex * frameResources->MaterialsBuffers.GetElementSize();
        cmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

        auto& uuid = mModels[i].GetUUID();
        auto instanceCount = mModels[i].PrepareInstances(frameResources->InstanceBuffer);
        
        cmdList->SetGraphicsRootShaderResourceView(3, frameResources->InstanceBuffer[uuid].GetGPUVirtualAddress());

        if (objectMaterial->GetTextureIndex() != -1)
        {
            auto textureSRVResult = textureManager->GetGPUDescriptorSrvHandleForTextureIndex(objectMaterial->GetTextureIndex());
            cmdList->SetGraphicsRootDescriptorTable(
                4, textureSRVResult.Get());
        }
        cmdList->DrawIndexedInstanced(mModels[i].GetIndexCount(),
            instanceCount, // Number of instances
            mModels[i].GetStartIndexLocation(),
            mModels[i].GetBaseVertexLocation(),
            0); // Start InstanceLocation
    }
}

void Application::ResetModelsInstances()
{
    for (auto& model : mModels)
    {
        model.ResetCurrentInstances();
    }
}
