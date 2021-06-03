// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct  FPerlinNoise2DParameters
{
	UTextureRenderTarget2D* RenderTarget;


	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FPerlinNoise2DParameters() { }
	FPerlinNoise2DParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
public:
	uint32 TimeStamp;
};

class COMPUTESHADERS_API FPerlinNoise2DManager
{
public:
	static FPerlinNoise2DManager* Get()
	{
		if (!Instance)
			Instance = new FPerlinNoise2DManager();
		return Instance;
	}
	
	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();
	
	// Call this whenever you have new parameters to share.
	void UpdateParameters(FPerlinNoise2DParameters& DrawParameters);

private:
	//Private constructor to prevent client from instantiating
	FPerlinNoise2DManager() = default;
	
	//The singleton instance
	static FPerlinNoise2DManager* Instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FPerlinNoise2DParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
	public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};
