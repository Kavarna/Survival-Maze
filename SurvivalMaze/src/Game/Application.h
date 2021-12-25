#pragma once


#include "Engine.h"
#include "Maze.h"
#include "CompositeModel.h"



class Application : public Engine
{
public:
    Application();
    ~Application() = default;

public:
    // Inherited via Engine
    virtual bool OnInit(ID3D12GraphicsCommandList* initializationCmdList, ID3D12CommandAllocator* cmdAllocator) override;
    virtual bool OnUpdate(FrameResources* frameResources, float dt) override;
    virtual bool OnRender(ID3D12GraphicsCommandList* cmdList, FrameResources* frameResources) override;
    virtual bool OnRenderGUI() override;
    virtual bool OnResize() override;
    virtual std::unordered_map<uuids::uuid, uint32_t> GetInstanceCount() override;

    virtual uint32_t GetModelCount() override;
    virtual ID3D12PipelineState* GetBeginFramePipeline() override;

private:
    bool InitModels(ID3D12GraphicsCommandList* initializationCmdList, ID3D12CommandAllocator* cmdAllocator);
    bool InitPlayerModel();

private:
    void ReactToKeyPresses(float dt);
    void UpdateCamera(FrameResources* frameResources);
    void UpdateModels(FrameResources* frameResources);

    void RenderModels(ID3D12GraphicsCommandList* cmdList, FrameResources* frameResources);

    void ResetModelsInstances();

private:
    std::vector<Model> mModels;
    Model* mCubeModel;

    CompositeModel mPlayer;

    SceneLight mSceneLight;

    Maze mMaze;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT mScissors;

    Camera mCamera;

    bool mMenuActive = true;

};
