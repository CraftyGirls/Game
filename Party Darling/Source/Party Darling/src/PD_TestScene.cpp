#pragma once

#include <PD_TestScene.h>
#include <PD_Game.h>
#include <PD_ResourceManager.h>
#include <PD_Player.h>
#include <PD_ContactListener.h>

#include <MeshEntity.h>
#include <MeshInterface.h>
#include <MeshFactory.h>
#include <Resource.h>

#include <DirectionalLight.h>
#include <PointLight.h>
#include <Material.h>

#include <shader\BaseComponentShader.h>
#include <shader\ShaderComponentTexture.h>
#include <shader\ShaderComponentDiffuse.h>
#include <shader\ShaderComponentPhong.h>
#include <shader\ShaderComponentBlinn.h>
#include <shader\ShaderComponentShadow.h>
#include <shader\ShaderComponentHsv.h>

#include <Box2DWorld.h>
#include <Box2DMeshEntity.h>
#include <Box2DDebugDraw.h>

#include <MousePerspectiveCamera.h>
#include <FollowCamera.h>

#include <Sound.h>
#include <libzplay.h>

#include <System.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <GLFW\glfw3.h>
#include <MatrixStack.h>

#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <NumberUtils.h>

PD_TestScene::PD_TestScene(Game * _game) :
	Scene(_game),
	shader(new BaseComponentShader(true)),
	world(new Box2DWorld(b2Vec2(0, 0))),
	drawer(nullptr),
	player(nullptr),
	sceneHeight(150),
	sceneWidth(50),
	firstPerson(true),
	screenSurfaceShader(new Shader("../assets/RenderSurface", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader)),
	screenFBO(new StandardFrameBuffer(true)),
	phongMat(new Material(15.0, glm::vec3(1.f, 1.f, 1.f), true)),
	hsvComponent(new ShaderComponentHsv(shader, 0, 1, 1)),
	joy(new JoystickManager()),
	uiLayer(0,0,0,0)
{
	shader->components.push_back(new ShaderComponentTexture(shader));
	shader->components.push_back(new ShaderComponentDiffuse(shader));
	shader->components.push_back(hsvComponent);
	//shader->components.push_back(new ShaderComponentPhong(shader));
	//shader->components.push_back(new ShaderComponentBlinn(shader));
	//shader->components.push_back(new ShaderComponentShadow(shader));
	shader->compileShader();

	//Set up cameras
	mouseCam = new MousePerspectiveCamera();
	cameras.push_back(mouseCam);
	mouseCam->farClip = 1000.f;
	mouseCam->nearClip = 0.001f;
	mouseCam->transform->rotate(90, 0, 1, 0, kWORLD);
	mouseCam->transform->translate(5.0f, 1.5f, 22.5f);
	mouseCam->yaw = 90.0f;
	mouseCam->pitch = -10.0f;
	mouseCam->speed = 1;

	debugCam = new MousePerspectiveCamera();
	cameras.push_back(debugCam);
	debugCam->farClip = 1000.f;
	debugCam->transform->rotate(90, 0, 1, 0, kWORLD);
	debugCam->transform->translate(5.0f, 1.5f, 22.5f);
	debugCam->yaw = 90.0f;
	debugCam->pitch = -10.0f;
	debugCam->speed = 1;

	gameCam = new FollowCamera(15, glm::vec3(0, 0, 0), 0, 0);
	cameras.push_back(gameCam);
	gameCam->farClip = 1000.f;
	gameCam->transform->rotate(90, 0, 1, 0, kWORLD);
	gameCam->transform->translate(5.0f, 1.5f, 22.5f);
	gameCam->minimumZoom = 22.5f;
	gameCam->yaw = 90.0f;
	gameCam->pitch = -10.0f;

	activeCamera = mouseCam;
	
	float _size = 3;
	std::vector<Box2DMeshEntity *> boundaries;
	MeshInterface * boundaryMesh = MeshFactory::getCubeMesh();
	boundaries.push_back(new Box2DMeshEntity(world, boundaryMesh, b2_staticBody));
	boundaries.push_back(new Box2DMeshEntity(world, boundaryMesh, b2_staticBody));
	boundaries.push_back(new Box2DMeshEntity(world, boundaryMesh, b2_staticBody));
	boundaries.push_back(new Box2DMeshEntity(world, boundaryMesh, b2_staticBody));

	boundaries.at(0)->transform->scale(_size, sceneHeight*0.5f + _size*2.f, _size * 4.f);
	boundaries.at(1)->transform->scale(_size, sceneHeight*0.5f + _size*2.f, _size * 4.f);
	boundaries.at(2)->transform->scale(sceneWidth*0.5f + _size*2.f, _size, _size * 4.f);
	boundaries.at(3)->transform->scale(sceneWidth*0.5f + _size*2.f, _size, _size * 4.f);

	boundaries.at(0)->setTranslationPhysical(sceneWidth+_size, sceneHeight*0.5f, 0);
	boundaries.at(1)->setTranslationPhysical(-_size, sceneHeight*0.5f, 0);
	boundaries.at(2)->setTranslationPhysical(sceneWidth*0.5f, sceneHeight+_size, 0);
	boundaries.at(3)->setTranslationPhysical(sceneWidth*0.5f, -_size, 0);
	
	b2Filter sf;
	//sf.categoryBits = PuppetGame::kBOUNDARY;
	//sf.maskBits = -1;
	for(auto b : boundaries){
		addChild(b);
		b->setShader(shader, true);
		world->addToWorld(b);
		b->body->GetFixtureList()->SetFilterData(sf);
		b->mesh->pushMaterial(phongMat);
		//b->mesh->pushTexture2D(PuppetResourceManager::stageFront);
	}
	//sf.categoryBits = PuppetGame::kBOUNDARY | PuppetGame::kGROUND;
	boundaries.at(3)->body->GetFixtureList()->SetFilterData(sf);
	boundaries.at(3)->body->GetFixtureList()->SetFriction(1);
	boundaries.at(3)->body->GetFixtureList()->SetRestitution(0);

	MeshEntity * ground = new MeshEntity(MeshFactory::getPlaneMesh());
	ground->transform->translate(sceneWidth/2.f, sceneHeight/2.f, -2.f);
	ground->transform->scale(sceneWidth/2.f, sceneHeight/2.f, 1);
	ground->setShader(shader, true);
	addChild(ground);

	/*MeshEntity * ceiling = new MeshEntity(MeshFactory::getPlaneMesh());
	ceiling->transform->translate(sceneWidth/2.f, sceneHeight/2.f, _size * 4.f);
	ceiling->transform->scale(sceneWidth, sceneHeight, 1);
	ceiling->setShader(shader, true);
	addChild(ceiling);*/


	//lights.push_back(new DirectionalLight(glm::vec3(1,0,0), glm::vec3(1,1,1), 0));
	
	player = new PD_Player(world);
	player->setShader(shader, true);
	gameCam->addTarget(player, 1);
	addChild(player);
	player->setTranslationPhysical(sceneWidth / 2.f, sceneHeight / 8.f, 0, false);
	player->body->SetLinearDamping(2.5f);
	player->body->SetAngularDamping(2.5f);

	//intialize key light
	PointLight * keyLight = new PointLight(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(1.f, 1.f, 1.f), 0.00f, 0.01f, -10.f);
	//Set it as the key light so it casts shadows
	//keyLight->isKeyLight = true;
	//Add it to the scene
	lights.push_back(keyLight);
	player->addChild(keyLight);
	
	mouseCam->upVectorLocal = glm::vec3(0, 0, 1);
	mouseCam->forwardVectorLocal = glm::vec3(1, 0, 0);
	mouseCam->rightVectorLocal = glm::vec3(0, -1, 0);

	PD_ContactListener * cl = new PD_ContactListener(this);
	world->b2world->SetContactListener(cl);
	
	crosshair = new Sprite();
	crosshair->mesh->pushTexture2D(PD_ResourceManager::crosshair);
	crosshair->transform->scale(8,8,1);
	uiLayer.addChild(crosshair);

	playerIndicator = new Sprite();
	playerIndicator->mesh->pushTexture2D(PD_ResourceManager::crosshair);
	playerIndicator->transform->scale(8,8,1);
	uiLayer.addChild(playerIndicator);

	mouseIndicator = new Sprite();
	mouseIndicator->mesh->pushTexture2D(PD_ResourceManager::crosshair);
	mouseIndicator->transform->scale(8,8,1);
	uiLayer.addChild(mouseIndicator);
}

PD_TestScene::~PD_TestScene(){
	while(children.size() > 0){
		NodeHierarchical::deleteRecursively(children.back());
		children.pop_back();
	}
	
	shader->safeDelete();
	//delete phongMat;
	delete world;

	delete screenSurface;
	//screenSurfaceShader->safeDelete();
	screenFBO->safeDelete();
	delete joy;
}

void PD_TestScene::update(Step * _step){
	joy->update(_step);

	if(keyboard->keyJustUp(GLFW_KEY_F11)){
		game->toggleFullScreen();
	}

	if(keyboard->keyJustUp(GLFW_KEY_F)){
		firstPerson = !firstPerson;
	}

	// camera controls
	if (keyboard->keyDown(GLFW_KEY_UP)){
		activeCamera->transform->translate((activeCamera->forwardVectorRotated) * static_cast<MousePerspectiveCamera *>(activeCamera)->speed);
	}
	if (keyboard->keyDown(GLFW_KEY_DOWN)){
		activeCamera->transform->translate((activeCamera->forwardVectorRotated) * -static_cast<MousePerspectiveCamera *>(activeCamera)->speed);
	}
	if (keyboard->keyDown(GLFW_KEY_LEFT)){
		activeCamera->transform->translate((activeCamera->rightVectorRotated) * -static_cast<MousePerspectiveCamera *>(activeCamera)->speed);
	}
	if (keyboard->keyDown(GLFW_KEY_RIGHT)){
		activeCamera->transform->translate((activeCamera->rightVectorRotated) * static_cast<MousePerspectiveCamera *>(activeCamera)->speed);
	}

	if(firstPerson){
		float playerSpeed = 2.5f;
		float mass = player->body->GetMass();
		float angle = atan2(mouseCam->forwardVectorRotated.y, mouseCam->forwardVectorRotated.x);

		if(activeCamera != mouseCam){
			angle = glm::radians(90.f);
		}

		mouseCam->transform->translate(player->getPos(false) + glm::vec3(0, 0, player->transform->getScaleVector().z*1.25f), false);
		mouseCam->lookAtOffset = glm::vec3(0, 0, -player->transform->getScaleVector().z*0.25f);
		
		
		if (keyboard->keyDown(GLFW_KEY_W)){
			player->applyLinearImpulseUp(playerSpeed * mass * sin(angle));
			player->applyLinearImpulseRight(playerSpeed * mass * cos(angle));
		}
		if (keyboard->keyDown(GLFW_KEY_S)){
			player->applyLinearImpulseDown(playerSpeed * mass * sin(angle));
			player->applyLinearImpulseLeft(playerSpeed * mass * cos(angle));
		}
		if (keyboard->keyDown(GLFW_KEY_A)){
			player->applyLinearImpulseUp(playerSpeed * mass * cos(angle));
			player->applyLinearImpulseLeft(playerSpeed * mass * sin(angle));
		}
		if (keyboard->keyDown(GLFW_KEY_D)){
			player->applyLinearImpulseDown(playerSpeed * mass * cos(angle));
			player->applyLinearImpulseRight(playerSpeed * mass * sin(angle));
		}
		
		// correct joystick controls for first-person
		Joystick * one = joy->joysticks[0];
			if(one != nullptr){
			float x = playerSpeed * mass * cos(angle) * -one->getAxis(Joystick::xbox_axes::kLY) +
				playerSpeed * mass * sin(angle) * one->getAxis(Joystick::xbox_axes::kLX);
			float y = playerSpeed * mass * sin(angle) * -one->getAxis(Joystick::xbox_axes::kLY) +
				playerSpeed * mass * cos(angle) * -one->getAxis(Joystick::xbox_axes::kLX);

			player->applyLinearImpulseUp(y);
			player->applyLinearImpulseRight(x);
		}
	}

	// debug controls
	if(keyboard->keyJustDown(GLFW_KEY_1)){
		if(activeCamera == gameCam){
			activeCamera = mouseCam;
		}else if(activeCamera == mouseCam){
			activeCamera = debugCam;
		}else{
			activeCamera = gameCam;
		}
	}
	if(keyboard->keyJustUp(GLFW_KEY_2)){
		if(drawer != nullptr){
			world->b2world->SetDebugDraw(nullptr);
			removeChild(drawer);
			delete drawer;
			drawer = nullptr;
		}else{
			drawer = new Box2DDebugDraw(world);
			world->b2world->SetDebugDraw(drawer);
			drawer->drawing = true;
			//drawer->AppendFlags(b2Draw::e_aabbBit);
			drawer->AppendFlags(b2Draw::e_shapeBit);
			drawer->AppendFlags(b2Draw::e_centerOfMassBit);
			drawer->AppendFlags(b2Draw::e_jointBit);
			//drawer->AppendFlags(b2Draw::e_pairBit);
			addChild(drawer);
		}
	}
	
	Scene::update(_step);
	world->update(_step);

	glm::uvec2 sd = vox::getScreenDimensions();
	uiLayer.resize(0, sd.x, 0, sd.y);
	uiLayer.update(_step);

	glm::vec3 pos = player->getPos(false);
	std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;

	glm::vec2 newPos = activeCamera->worldToScreen(pos, sd);

	std::cout << newPos.x << " " << newPos.y << std::endl;
	std::cout << std::endl;
	playerIndicator->transform->translate(newPos.x, newPos.y, 0, false);

	crosshair->transform->translate(sd.x/2.f, sd.y/2.f, 0, false);

	mouseIndicator->transform->translate(sd.x - mouse->mouseX(), sd.y - mouse->mouseY(), 0, false);
}

void PD_TestScene::render(vox::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	screenFBO->resize(game->viewPortWidth, game->viewPortHeight);
	//Bind frameBuffer
	screenFBO->bindFrameBuffer();
	//render the scene to the buffer
	Scene::render(_matrixStack, _renderOptions);
	uiLayer.render(_matrixStack, _renderOptions);
	//Render the buffer to the render surface
	screenSurface->render(screenFBO->getTextureId());
}

void PD_TestScene::load(){
	Scene::load();	

	screenSurface->load();
	screenFBO->load();
}

void PD_TestScene::unload(){
	Scene::unload();	

	screenSurface->unload();
	screenFBO->unload();
}