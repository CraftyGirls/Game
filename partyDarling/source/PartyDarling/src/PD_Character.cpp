#pragma once

#include <PD_Character.h>
#include <PD_Assets.h>

#include <MeshInterface.h>
#include <MeshFactory.h>
#include <PD_ResourceManager.h>
#include <PD_Assets.h>
#include <PD_Character.h>
#include <NumberUtils.h>
#include <TextureColourTable.h>

#include <sweet/Input.h>

PersonButt::PersonButt(BulletWorld * _world, PersonRenderer * _person) :
	NodeBulletBody(_world),
	person(_person)
{

}

Person::Person(BulletWorld * _world, AssetCharacter * const _definition, MeshInterface * _mesh, Anchor_t _anchor):
	RoomObject(_world, _mesh, _anchor),
	pr(new PersonRenderer(_world, _definition)) // TODO: fix this
{
	setColliderAsCapsule(0.5f,1.f);
	createRigidBody(25);
	body->setAngularFactor(btVector3(0,1,0)); // prevent from falling over
	meshTransform->setVisible(false);

	childTransform->addChild(pr)->scale(0.001);
}

void Person::setShader(Shader * _shader, bool _configureDefault){
	RoomObject::setShader(_shader, _configureDefault);
	pr->setShader(_shader, _configureDefault);
}

PersonComponent::PersonComponent(CharacterComponentDefinition * const _definition, Texture * _paletteTex, bool _flipped) :
	Sprite(),
	flipped(_flipped)
{
	// get texture
	AssetTexture * tex = PD_ResourceManager::componentTextures->getTexture(_definition->texture);
	tex->load();

	// apply palette + texture
	mesh->pushTexture2D(_paletteTex);
	mesh->pushTexture2D(tex->texture);
	
	in = _definition->in;
	out = _definition->out;
	// handle flipping
	if(flipped){
		meshTransform->scale(-1, 1, 1, false);
		in.x = 1 - in.x;
		for(glm::vec2 & o : out){
			o.x = 1 - o.x;
		}
	}
	
	// multiply percentage coordinates by width/height to corresponding to specific texture
	in.x *= tex->texture->width;
	in.y *= tex->texture->height;
	for(glm::vec2 & o : out){
		o.x *= tex->texture->width;
		o.y *= tex->texture->height;
	}
	
	// scale and translate the mesh into position
	meshTransform->scale(tex->texture->width, tex->texture->height, 1);
	meshTransform->translate(tex->texture->width*0.5f -in.x, tex->texture->height*0.5f -in.y, 0, false);

	mesh->setScaleMode(GL_NEAREST);
}

glm::vec2 PersonComponent::getOut(unsigned long int _index){
	return (out.size() > 0 ? out.at(_index) : glm::vec2(0,0)) - in;
}

PersonLimbSolver::PersonLimbSolver(glm::vec2 _pos) :
	IkChain_CCD(_pos)
{
}

void PersonLimbSolver::addComponent(PersonComponent * _component, float _weight){
	AnimationJoint * j = new AnimationJoint(_component->getOut(0));
	jointsLocal.back()->childTransform->addChild(_component);
	addJointToChain(j);
	components.push_back(_component);
}

PersonState::PersonState(Json::Value _json) :
	name(_json.get("name", "NO_NAME").asString()),
	conversation(_json.get("convo", "NO_CONVO").asString())
{
}

PersonRenderer::PersonRenderer(BulletWorld * _world, AssetCharacter * const _definition) :
	paletteTex(new TextureColourTable(false)),
	timer(0),
	state(nullptr)
{
	paletteTex->load();
	
	CharacterComponentDefinition
		* pelvisDef			= &_definition->root,
		* torsoDef			= &pelvisDef->components.at(0),

		* jawDef			= &torsoDef->components.at(0),
		* headDef			= &jawDef->components.at(0),
		* noseDef			= &headDef->components.at(0),
		* eyebrowLDef		= &headDef->components.at(1),
		* eyebrowRDef		= &headDef->components.at(2),
		* eyeLDef			= &headDef->components.at(3),
		* eyeRDef			= &headDef->components.at(4),
		* pupilLDef			= &eyeLDef->components.at(0),
		* pupilRDef			= &eyeRDef->components.at(0),

		* armLDef			= &torsoDef->components.at(2),
		* armRDef			= &torsoDef->components.at(1),
		* forearmLDef		= &armLDef->components.at(0),
		* forearmRDef		= &armRDef->components.at(0),
		* handLDef			= &forearmLDef->components.at(0),
		* handRDef			= &forearmRDef->components.at(0),
		
		* legLDef			= &pelvisDef->components.at(2),
		* legRDef			= &pelvisDef->components.at(1),
		* forelegLDef		= &legLDef->components.at(0),
		* forelegRDef		= &legRDef->components.at(0),
		* footLDef			= &forelegLDef->components.at(0),
		* footRDef			= &forelegRDef->components.at(0);


	pelvis = new PersonComponent(pelvisDef, paletteTex, false);

	torso = new PersonComponent(torsoDef, paletteTex, false);

	jaw = new PersonComponent(jawDef, paletteTex, false);
	head = new PersonComponent(headDef, paletteTex, false);

	nose = new PersonComponent(noseDef, paletteTex, false);
	eyebrowL = new PersonComponent(eyebrowLDef, paletteTex, false);
	eyebrowR = new PersonComponent(eyebrowRDef, paletteTex, false);
	eyeL = new PersonComponent(eyeLDef, paletteTex, false);
	eyeR = new PersonComponent(eyeRDef, paletteTex, false);
	pupilL = new PersonComponent(pupilLDef, paletteTex, false);
	pupilR = new PersonComponent(pupilRDef, paletteTex, false);

	armR = new PersonComponent(armRDef, paletteTex, true);
	forearmR = new PersonComponent(forearmRDef, paletteTex, true);
	handR = new PersonComponent(handRDef, paletteTex, true);

	armL = new PersonComponent(armLDef, paletteTex, false);
	forearmL = new PersonComponent(forearmLDef, paletteTex, false);
	handL = new PersonComponent(handLDef, paletteTex, false);

	legR = new PersonComponent(legRDef, paletteTex, true);
	forelegR = new PersonComponent(forelegRDef, paletteTex, true);
	footR = new PersonComponent(footRDef, paletteTex, true);
					
	legL = new PersonComponent(legLDef, paletteTex, false);
	forelegL = new PersonComponent(forelegLDef, paletteTex, false);
	footL = new PersonComponent(footLDef, paletteTex, false);

	solverArmR = new PersonLimbSolver(torso->getOut(1));
	solverArmL = new PersonLimbSolver(torso->getOut(2));
	solverLegR = new PersonLimbSolver(pelvis->getOut(1));
	solverLegL = new PersonLimbSolver(pelvis->getOut(2));
	solverBod = new PersonLimbSolver(glm::vec2(0));

	// implicitly create skeletal structure by adding components in the correct order
	solverArmR->addComponent(armR);
	solverArmR->addComponent(forearmR);
	connect(forearmR, handR);
	//solverArmR->addComponent(handR);
	
	solverArmL->addComponent(armL);
	solverArmL->addComponent(forearmL);
	connect(forearmL, handL);
	//solverArmL->addComponent(handL);
	
	solverLegR->addComponent(legR);
	solverLegR->addComponent(forelegR);
	connect(forelegR, footR);
	//solverLegR->addComponent(footR);
	
	solverLegL->addComponent(legL);
	solverLegL->addComponent(forelegL);
	connect(forelegL, footL);
	//solverLegL->addComponent(footL);
	
	solverBod->addComponent(pelvis);
	solverBod->addComponent(torso);
	connect(torso, jaw);
	connect(jaw, head);
	//solverBod->addComponent(jaw);
	//solverBod->addComponent(head);

	// no point in putting the nose/eyes into the skeletal structure
	connect(head, nose);
	connect(head, eyebrowL);
	connect(head, eyebrowR);
	connect(head, eyeL);
	connect(head, eyeR);
	connect(eyeL, pupilL);
	connect(eyeR, pupilR);
	
	// attach the arms/legs to the spine
	solverBod->jointsLocal.at(1)->addJoint(solverArmR);
	solverBod->jointsLocal.at(1)->addJoint(solverArmL);
	solverBod->jointsLocal.at(0)->addJoint(solverLegR);
	solverBod->jointsLocal.at(0)->addJoint(solverLegL);
	childTransform->addChild(solverBod, false);

	
	solvers.push_back(solverBod);
	solvers.push_back(solverArmR);
	solvers.push_back(solverArmL);
	solvers.push_back(solverLegR);
	solvers.push_back(solverLegL);
	currentSolver = solvers.front();
	
	// pre-initialize the effectors to a T-pose type thing
	solverArmR->target = glm::vec2(-solverArmR->getChainLength(), 0);
	solverArmL->target = glm::vec2(solverArmL->getChainLength(), 0);
	solverLegR->target = glm::vec2(0, -solverLegR->getChainLength());
	solverLegL->target = glm::vec2(0, -solverLegL->getChainLength());
	solverBod->target = glm::vec2(0, solverBod->getChainLength());
	
	
	// talking thing
	talkHeight = head->parents.at(0)->getTranslationVector().y;
	talk = new Animation<float>(&talkHeight);
	talk->tweens.push_back(new Tween<float>(0.1, head->mesh->textures.at(1)->height*0.4, Easing::kEASE_IN_OUT_CIRC));
	talk->tweens.push_back(new Tween<float>(0.1, -head->mesh->textures.at(1)->height*0.4, Easing::kEASE_IN_OUT_CIRC));
	talk->loopType = Animation<float>::LoopType::kLOOP;
	talk->hasStart = true;


	butt = new PersonButt(_world, this);
	childTransform->addChild(butt);
	butt->setColliderAsBox((torso->getOut(2).x - torso->getOut(1).x) * 0.005f, solverBod->getChainLength() * 0.005f, 0.005f); // TODO: make this scale factor not hard-coded
	butt->createRigidBody(0);
}

PersonRenderer::~PersonRenderer(){
	delete paletteTex;
}

void PersonRenderer::connect(PersonComponent * _from, PersonComponent * _to, bool _behind){
	joints.push_back(_from->childTransform->addChild(_to));
	joints.back()->translate(
		_from->out.at(_from->connections.size()).x - _from->in.x,
		_from->out.at(_from->connections.size()).y - _from->in.y,
		0, false);
	_from->connections.push_back(_to);
}

void PersonRenderer::setShader(Shader * _shader, bool _default){
	pelvis->setShader(_shader, _default);
	torso->setShader(_shader, _default);
	jaw->setShader(_shader, _default);
	head->setShader(_shader, _default);
	
	nose->setShader(_shader, _default);
	eyebrowL->setShader(_shader, _default);
	eyebrowR->setShader(_shader, _default);
	eyeL->setShader(_shader, _default);
	eyeR->setShader(_shader, _default);
	pupilL->setShader(_shader, _default);
	pupilR->setShader(_shader, _default);

	armL->setShader(_shader, _default);
	forearmL->setShader(_shader, _default);
	handL->setShader(_shader, _default);

	armR->setShader(_shader, _default);
	forearmR->setShader(_shader, _default);
	handR->setShader(_shader, _default);

	legL->setShader(_shader, _default);
	forelegL->setShader(_shader, _default);
	footL->setShader(_shader, _default);

	legR->setShader(_shader, _default);
	forelegR->setShader(_shader, _default);
	footR->setShader(_shader, _default);
}

void PersonRenderer::update(Step * _step){
	

	timer += _step->deltaTime;

	if(timer > 1){
		timer = 0;
		float l;

		l = solverArmR->getChainLength();
		solverArmR->target.x = sweet::NumberUtils::randomFloat(-l, 0);
		solverArmR->target.y = sweet::NumberUtils::randomFloat(-l, l);
		
		l = solverArmL->getChainLength();
		solverArmL->target.x = sweet::NumberUtils::randomFloat(l, 0);
		solverArmL->target.y = sweet::NumberUtils::randomFloat(-l, l);
		
		l = solverLegR->getChainLength();
		solverLegR->target.x = sweet::NumberUtils::randomFloat(0, l*0.5);
		solverLegR->target.y = sweet::NumberUtils::randomFloat(-l, -l*0.8);
		
		l = solverLegL->getChainLength();
		solverLegL->target.x = sweet::NumberUtils::randomFloat(-l*0.5, 0);
		solverLegL->target.y = sweet::NumberUtils::randomFloat(-l, -l*0.8);
		
		l = solverBod->getChainLength();
		solverBod->target.x = sweet::NumberUtils::randomFloat(-l*0.5, l*0.5);
		solverBod->target.y = sweet::NumberUtils::randomFloat(l*0.95, l);

		/*solverArmL->target = glm::vec2(solverArmL->getChainLength(), 0);
		solverLegR->target = glm::vec2(0, -solverLegR->getChainLength());
		solverLegL->target = glm::vec2(0, -solverLegL->getChainLength());
		solverBod->target = glm::vec2(0, solverBod->getChainLength());
		
		for(unsigned long int s = 1; s < solvers.size(); ++s){
			float l = solvers.at(s)->getChainLength();
			solvers.at(s)->target.x = sweet::NumberUtils::randomFloat(-l, l);
			solvers.at(s)->target.y = sweet::NumberUtils::randomFloat(-l, l);
		}*/
	}


	Keyboard & k = Keyboard::getInstance();

	glm::vec2 test(0);
	if(k.keyDown(GLFW_KEY_I)){
		test.y += 50;
	}if(k.keyDown(GLFW_KEY_J)){
		test.x -= 50;
	}if(k.keyDown(GLFW_KEY_K)){
		test.y -= 50;
	}if(k.keyDown(GLFW_KEY_L)){
		test.x += 50;
	}

	currentSolver->target += test;
	
	if(k.keyJustDown(GLFW_KEY_U)){
		if(currentSolver == solvers.back()){
			currentSolver = solvers.at(0);
		}else{
			for(unsigned long int i = 0; i < solvers.size()-1; ++i){
				if(currentSolver == solvers.at(i)){
					currentSolver = solvers.at(i+1);
					break;
				}
			}
		}
	}
	
	// talking
	talk->update(_step);
	glm::vec3 v = head->parents.at(0)->getTranslationVector();
	head->parents.at(0)->translate(v.x, talkHeight, v.z, false);

	Entity::update(_step);
}