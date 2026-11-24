#include "pch.h"
#include "PortObject.h"

#include "BoxCollider.h"
#include "Input.h"
#include "CollisionManager.h"
#include "Debug.h"
#include "World.h"
#include "BitmapComponent.h"
#include "AnimationComponent.h"
#include "D2DRenderer.h"
#include "FireObject.h"
#include "Sound.h"

PortObject::PortObject()
{
	m_tag = "port";
	// 139000 8400
	m_radius =320;

	m_pPortCollider = AddComponent<BoxCollider>("box");
	m_pPortCollider->SetExtends(30, 35);
	m_pPortCollider->SetCollisionType(CollisionType::UNBLOCK);
	m_pPortCollider->SetColliderGroup(ColliderGroup::ITEM);
	CollisionManager::GetInstance()->AddCollider(m_pPortCollider);



	m_pPortImage = AddComponent<BitmapComponent>("bitmap");
	m_pPortImage->SetBitmap(L"../Resource/image/polyport.png");

	m_pShadowTransform = AddComponent<TransformComponent>("transform");
	m_pShadowTransform->SetParentTransform(nullptr);

	m_pShadowAnimation = new AnimationComponent(this, "AnimationComponent", m_pShadowTransform);
	m_pShadowAnimation->SetAnimationAsset(L"../Resource/FSM/Port.txt");
	m_pShadowAnimation->SetAnimation("Blue", false, true);
	m_pComponents.push_back(m_pShadowAnimation);

	m_pShadowCollider = new BoxCollider(this, "shadowCollider", m_pShadowTransform);
	m_pShadowCollider->SetExtends(30, 30);
	m_pShadowCollider->SetColliderGroup(ColliderGroup::DESTINATION);
	CollisionManager::GetInstance()->AddCollider(m_pShadowCollider);
	m_pComponents.push_back(m_pShadowCollider);
	if (m_pBrush == nullptr)
	{
		D2DRenderer::m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrush);
	}
}

PortObject::~PortObject()
{
	if (m_pBrush != nullptr)
	{
		m_pBrush->Release();
		m_pBrush = nullptr;
	}
	
}

bool PortObject::Initialize()
{
	m_pPlayer = GetOwnerWorld()->GetPlayerObject();
	
	return __super::Initialize();
}

void PortObject::Update(float dt)
{
	__super::Update(dt);
	m_bActive = false;
	

	
}

void PortObject::LateUpdate(float dt)
{
	if (!m_bVisible) return;
	if (!m_bActive) return;



	m_player = m_pPlayer->GetRootComponent()->GetRelativeTranslation();
	m_player.y += 40;

	m_pos = m_pRootComponent->GetRelativeTranslation();
	Vector2D dif = m_pos - m_player;
	Vector2D dir = dif.Normalize();

	m_des = m_pos + dir * m_radius;

	m_pShadowTransform->SetRelativeTranslation(m_pos + dir * m_radius);

	double angle_rad = std::acos(dir.x / dir.Length());
	double angle_deg = angle_rad * 180.0 / 3.14;
	if (dir.y < 0) angle_deg = 360.0 - angle_deg;

	m_pRootComponent->SetRelativeRotation(angle_deg + 180);

	m_bCurCollision = m_pShadowCollider->GetFireCollision();


	if (!m_bCurCollision)
	{
		m_pShadowAnimation->SetAnimation("Blue", false, true);
		if (Input::GetInstance()->GetKeyState(KEY::E) == KEY_STATE::TAP)
		{
			m_pPlayer->GetRootComponent()->SetRelativeTranslation(m_des);
			Sound::Play(SFX, "../Resource/sound/interact.mp3");
		}
	}
	else if (m_bCurCollision)
	{
		m_pShadowAnimation->SetAnimation("Red", false, true);
	}

	Debug::Text(L"poly port : " + to_wstring((int)m_des.x) + L" " + to_wstring((int)m_des.y));

	__super::LateUpdate(dt);
}

void PortObject::Render(ID2D1HwndRenderTarget* pRenderTarget)
{
	m_pPortImage->Render(pRenderTarget);

	if (!m_bVisible) return;
	if (!m_bActive) return;

	Vector2D dif = m_pos - m_player;
	Vector2D dir = dif.Normalize();
	Vector2D tmp = m_pos - dir * m_radius;

	//ID2D1SolidColorBrush* pBrush = D2DRenderer::m_instance->GetBrush();
	D2DRenderer::m_pRenderTarget->SetTransform(World::m_cameraTransform);
	D2DRenderer::m_pRenderTarget->DrawLine({ tmp.x,tmp.y }, { m_des.x,m_des.y }, m_pBrush);

	D2D1_ELLIPSE circle;
	circle.point = { 0,0 };
	circle.radiusX = m_radius;
	circle.radiusY = m_radius;
	D2DRenderer::m_pRenderTarget->SetTransform(m_pRootComponent->GetWorldTransform() * World::m_cameraTransform);
	D2DRenderer::m_pRenderTarget->DrawEllipse(circle, m_pBrush);

	__super::Render(pRenderTarget);

	m_bActive = false;
}


