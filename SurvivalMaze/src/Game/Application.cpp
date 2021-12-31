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
    mProjectileManager.Update(dt);
    mMaze.Update(dt);
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
    mMaze.RenderDebug(frameResources->VertexBatchRenderer);
    mPlayer.Render();
    mPlayer.RenderDebug(frameResources->VertexBatchRenderer);
    mProjectileManager.Render();

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderModels(cmdList, frameResources);

    frameResources->VertexBatchRenderer.SetPipeline<PipelineType::DebugPipeline>(cmdList);
    frameResources->VertexBatchRenderer.End(cmdList);

    RenderHUD(cmdList, frameResources);
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

    DirectX::XMFLOAT3 camPos;
    DirectX::XMStoreFloat3(&camPos, mFirstPersonCamera.GetPosition());
    mFirstPersonCamera.Create(camPos, (float)mClientWidth / mClientHeight);
    mThirdPersonCamera.Create((float)mClientWidth / mClientHeight);
    mOrthograficCamera.Create({ 0.0f, 0.0f, 0.0f }, (float)mClientWidth, (float)mClientHeight);
    return true;
}

std::unordered_map<uuids::uuid, uint32_t> Application::GetInstanceCount()
{
    std::unordered_map<uuids::uuid, uint32_t> result;
    for (const auto& model : mModels)
    {
        result[model->GetUUID()] = model->GetInstanceCount();
    }
    return result;
}

uint32_t Application::GetModelCount()
{
    return (uint32_t)mModels.size();
}

uint32_t Application::GetPassCount()
{
    // One for 3D pass, one for 2D pass
    return 2;
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

    CHECK(mCubeModel.Create(Direct3D::kBufferCount, 0, "Resources\\Cube.obj"), false, "Unable to load cube");
    mCubeModel.ClearInstances();
    mModels.push_back(&mCubeModel);


    CHECK(mSphereModel.Create(Direct3D::kBufferCount, 1, "Resources\\Sphere.obj"), false, "Unable to load cube");
    mSphereModel.ClearInstances();
    mModels.push_back(&mSphereModel);

    ComPtr<ID3D12Resource> intermediaryResources[2];
    CHECK(Model::InitBuffers(initializationCmdList, intermediaryResources), false, "Unable to initialize buffers for models");

    mFirstPersonCamera.Create({ 0.0f, 0.0f, 0.0f }, (float)mClientWidth / mClientHeight);
    mThirdPersonCamera.Create((float)mClientWidth / mClientHeight);
    CHECK(mPlayer.Create(&mCubeModel, &mMaze), false, "Unable to create player model");
    mPlayer.SetCamera(&mThirdPersonCamera);
    mActiveCamera = &mThirdPersonCamera;

    CHECK(mProjectileManager.Create(&mSphereModel, MaximumProjectiles), false, "Unable to initialize projectile manager");

    Maze::MazeInitializationInfo mazeInfo = {};
    mazeInfo.rows = Random::get(10, 20);
    mazeInfo.cols = Random::get(10, 20);
    mazeInfo.tileWidthDepth = 5.0f;
    mazeInfo.cubeModel = &mCubeModel;
    mazeInfo.enemyModel = &mSphereModel;
    auto startPositionResult = mMaze.Create(mazeInfo);
    CHECK(startPositionResult.Valid(), false, "Unable to create maze");

    // mPlayer.mPosition = DirectX::XMVectorSetY(mPlayer.mPosition);
    auto& startPosition = startPositionResult.Get();
    startPosition.y = mPlayer.mModel.GetHalfHeight() + 0.25f; // animation looks better if we offset the model by 0.25f
    mPlayer.mPosition = DirectX::XMLoadFloat3(&startPosition);
    mThirdPersonCamera.SetTarget(mPlayer.mPosition);

    CHECK_HR(initializationCmdList->Close(), false);
    d3d->Flush(initializationCmdList, mFence.Get(), ++mCurrentFrame);

    return true;
}

void Application::ReactToKeyPresses(float dt)
{
    static int lastScrollWheelValue = 0;
    static bool cameraChangePressed = false;
    auto kb = mKeyboard->GetState();
    auto mouse = mMouse->GetState();
    bool mPlayerMoved = false;

    if (kb.Escape)
    {
        PostQuitMessage(0);
    }

    if (mRemainingTime >= 0.0f || mPlayer.mHealth < 0.0f)
    {
        if (kb.Up || kb.W)
        {
            if (mPlayer.Walk(dt))
            {
                UpdateCameraTarget(mPlayer.mPosition);
                mPlayerMoved = true;
            }
        }
        if (!mPlayerMoved && (kb.Down || kb.S))
        {
            if (mPlayer.Walk(-dt))
            {
                UpdateCameraTarget(mPlayer.mPosition);
                mPlayerMoved = true;
            }
        }
        if (!mPlayerMoved && (kb.Right || kb.D))
        {
            if (mPlayer.Strafe(dt))
            {
                UpdateCameraTarget(mPlayer.mPosition);
                mPlayerMoved = true;
            }
        }
        if (!mPlayerMoved && (kb.Left || kb.A))
        {
            if (mPlayer.Strafe(-dt))
            {
                UpdateCameraTarget(mPlayer.mPosition);
                mPlayerMoved = true;
            }
        }
        if (!mPlayerMoved)
        {
            mPlayer.ResetAnimation();
        }

        static bool spacePressed = false;
        if (kb.Space && !spacePressed)
        {
            DirectX::XMVECTOR direction = mActiveCamera->GetDirection();
            direction = DirectX::XMVectorSetY(direction, 0.0f);
            direction = DirectX::XMVector3Normalize(direction);
            mProjectileManager.SpawnProjectile(mPlayer.mPosition, direction);
            spacePressed = true;
        }
        else if (!kb.Space)
        {
            spacePressed = false;
        }
        mRemainingTime -= dt;
    }

    if (!mMenuActive)
    {
        mouse.x = Math::clamp(mouse.x, -25, 25);
        mouse.y = Math::clamp(mouse.y, -25, 25);
        
        mThirdPersonCamera.Update(dt, (float)mouse.x, (float)mouse.y);
        mFirstPersonCamera.Update(dt, (float)mouse.x, (float)mouse.y);

        int scrollValue = mouse.scrollWheelValue - lastScrollWheelValue;
        mThirdPersonCamera.AdjustZoom((float)scrollValue * 0.01f);
    }
    else
    {
        mThirdPersonCamera.Update(dt, 0.0f, 0.0f);
        mFirstPersonCamera.Update(dt, 0.0f, 0.0f);
    }

    if (kb.LeftControl && !cameraChangePressed)
    {
        if (mActiveCamera == &mThirdPersonCamera)
        {
            mActiveCamera = &mFirstPersonCamera;
        }
        else
        {
            mActiveCamera = &mThirdPersonCamera;
        }
        cameraChangePressed = true;
    }
    else if (!kb.LeftControl)
    {
        cameraChangePressed = false;
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
    if (mActiveCamera->DirtyFrames > 0)
    {
        auto mappedMemory = frameResources->PerPassBuffers.GetMappedMemory();
        mappedMemory->View = DirectX::XMMatrixTranspose(mActiveCamera->GetView());
        mappedMemory->Projection = DirectX::XMMatrixTranspose(mActiveCamera->GetProjection());

        DirectX::XMStoreFloat3(&mappedMemory->CameraPosition, mActiveCamera->GetPosition());

        mActiveCamera->DirtyFrames--;
    }
    if (mOrthograficCamera.DirtyFrames > 0)
    {
        auto mappedMemory = frameResources->PerPassBuffers.GetMappedMemory(1);
        mappedMemory->View = DirectX::XMMatrixTranspose(mOrthograficCamera.GetView());
        mappedMemory->Projection = DirectX::XMMatrixTranspose(mOrthograficCamera.GetProjection());

        DirectX::XMStoreFloat3(&mappedMemory->CameraPosition, mOrthograficCamera.GetPosition());


        // Also update hud world matrix
        auto hudMemory = frameResources->HUDBuffers.GetMappedMemory();
        hudMemory->World = DirectX::XMMatrixIdentity();
        hudMemory->TexWorld = DirectX::XMMatrixIdentity();

        mOrthograficCamera.DirtyFrames--;
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
        const auto* objectMaterial = mModels[i]->GetMaterial();
        materialBufferAddress += (uint64_t)objectMaterial->ConstantBufferIndex * frameResources->MaterialsBuffers.GetElementSize();
        cmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

        auto& uuid = mModels[i]->GetUUID();
        auto instanceCount = mModels[i]->PrepareInstances(frameResources->InstanceBuffer);
        if (instanceCount == 0)
        {
            continue;
        }
        
        cmdList->SetGraphicsRootShaderResourceView(3, frameResources->InstanceBuffer[uuid].GetGPUVirtualAddress());

        if (objectMaterial->GetTextureIndex() != -1)
        {
            auto textureSRVResult = textureManager->GetGPUDescriptorSrvHandleForTextureIndex(objectMaterial->GetTextureIndex());
            cmdList->SetGraphicsRootDescriptorTable(
                4, textureSRVResult.Get());
        }
        cmdList->DrawIndexedInstanced(mModels[i]->GetIndexCount(),
            instanceCount, // Number of instances
            mModels[i]->GetStartIndexLocation(),
            mModels[i]->GetBaseVertexLocation(),
            0); // Start InstanceLocation
    }
}

void Application::RenderHUD(ID3D12GraphicsCommandList* cmdList, FrameResources* frameResources)
{
    auto& batchRenderer = frameResources->HUDBatchRenderer;
    batchRenderer.Begin();

    // batchRenderer.Rectangle({ -100.0f, -100.0f }, { 100.0f, 100.f }, { 1.0f, 1.0f, 0.0f, 1.0f });
    float mHalfScreenWidth = mClientWidth / 2.0f;
    float mHalfScreenHeight = mClientHeight / 2.0f;
    
    {
        // Time bar
        float barHeight = 30.f, barWidth = 300.0f;
        float left = -mHalfScreenWidth + 10, right = -mHalfScreenWidth + 10 + barWidth;
        float bottom = mHalfScreenHeight - 10 - barHeight;
        float top = mHalfScreenHeight - 10;
        batchRenderer.Rectangle({ left, bottom },
            { right, top }, { 1.0f, 1.0f, 0.0f, 1.0f });

        left += 5;
        right -= 5;
        bottom += 5;
        top -= 5;

        right = Math::LinearInterpolation(mRemainingTime / MaximumTime, left, right);

        batchRenderer.Rectangle({ left, bottom },
            { right, top }, { 0.5f, 0.0f, 0.5f, 1.0f });
    }
    {
        // Health bar
        float barHeight = 30.f, barWidth = 300.0f;
        float left = mHalfScreenWidth - 10 - barWidth, right = mHalfScreenWidth - 10;
        float bottom = mHalfScreenHeight - 10 - barHeight;
        float top = mHalfScreenHeight - 10;
        batchRenderer.Rectangle({ left, bottom },
                { right, top }, { 1.0f, 1.0f, 0.0f, 1.0f });

        left += 5;
        right -= 5;
        bottom += 5;
        top -= 5;

        right = Math::LinearInterpolation(mPlayer.mHealth, left, right);

        batchRenderer.Rectangle({ left, bottom },
            { right, top }, { 1.0f, 0.0f, 0.0f, 1.0f });
    }

    CHECKRET((batchRenderer.SetPipeline<PipelineType::SimpleColor, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST>(cmdList)),
        false, "Unable to set SimpleColor pipeline to the HUD batch renderer");

    auto perObjectBufferAddress = frameResources->HUDBuffers.GetGPUVirtualAddress();
    cmdList->SetGraphicsRootConstantBufferView(0, perObjectBufferAddress);

    auto perPassBufferAddress = frameResources->PerPassBuffers.GetGPUVirtualAddress();
    perPassBufferAddress += frameResources->PerPassBuffers.GetElementSize();
    cmdList->SetGraphicsRootConstantBufferView(1, perPassBufferAddress);

    batchRenderer.End(cmdList);
}

void Application::ResetModelsInstances()
{
    for (auto& model : mModels)
    {
        model->ResetCurrentInstances();
    }
}

void Application::UpdateCameraTarget(const DirectX::XMVECTOR& position)
{
    mFirstPersonCamera.SetPosition(position);
    mThirdPersonCamera.SetTarget(position);
}
