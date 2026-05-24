#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

/**
 * FStrategosStyle — constantes visuais derivadas do design system do mockup HTML/CSS.
 *
 * Módulo base: 8 px. Layout alvo: 1920×1080.
 * Tudo é múltiplo de 8 para manter coerência com o grid do mockup.
 *
 * Uso em Blueprint: não exposto diretamente — os widgets C++ consomem esses
 * valores e os aplicam via GetWidgetStyle / SlateBrush nos construtores de
 * estilo (UStrategosStyleSet, a criar em pós-MVP).
 *
 * Cores em FLinearColor(R, G, B, A) — sRGB linear, não gamma.
 */
namespace StrategosStyle
{
	// ── MÓDULO ───────────────────────────────────────────────────────────────
	static constexpr float M  = 8.f;   // módulo base
	static constexpr float M2 = 16.f;
	static constexpr float M3 = 24.f;
	static constexpr float M4 = 32.f;
	static constexpr float M6 = 48.f;
	static constexpr float M8 = 64.f;

	// ── LAYOUT ───────────────────────────────────────────────────────────────
	static constexpr float TopBarHeight      = 64.f;   // 8M
	static constexpr float LeftRailWidth     = 56.f;   // 7M
	static constexpr float RightOutlinerW    = 320.f;  // 40M
	static constexpr float ProvinceDockH     = 44.f;   // colapsada
	static constexpr float ProvinceDockHOpen = 560.f;  // expandida
	static constexpr float ProvinceDockW     = 760.f;
	static constexpr float BorderRadius      = 0.f;    // design usa 0-2px

	// ── CARTA DE UNIDADE ────────────────────────────────────────────────────
	static constexpr float CardCompactW  = 192.f;
	static constexpr float CardCompactH  = 288.f;
	static constexpr float CardMicroW    = 144.f;
	static constexpr float CardMicroH    = 320.f;
	static constexpr float CardDetailedW = 720.f;
	static constexpr float CardDetailedH = 1024.f;

	// ── TEMA GRAFITE INDUSTRIAL (padrão) ────────────────────────────────────
	namespace Graphite
	{
		static const FLinearColor Bg0(0.025f, 0.031f, 0.041f);  // #07090c
		static const FLinearColor Bg1(0.047f, 0.063f, 0.086f);  // #0f1623
		static const FLinearColor Bg2(0.122f, 0.149f, 0.188f);  // #1f2630
		static const FLinearColor Bg3(0.165f, 0.192f, 0.251f);  // #2a3140
		static const FLinearColor Bg4(0.212f, 0.239f, 0.302f);  // #363d4d

		static const FLinearColor Line(0.212f, 0.239f, 0.302f);       // #363d4d
		static const FLinearColor LineStrong(0.306f, 0.345f, 0.404f); // #4e5867
		static const FLinearColor LineSoft(0.165f, 0.192f, 0.251f);   // #2a3140

		static const FLinearColor Text(0.929f, 0.894f, 0.824f);       // #ece4d2 warm bone
		static const FLinearColor TextMute(0.647f, 0.624f, 0.553f);   // #a59f8d
		static const FLinearColor TextDim(0.435f, 0.416f, 0.369f);    // #6f6a5e
		static const FLinearColor TextFaint(0.290f, 0.278f, 0.247f);  // #4a473f

		static const FLinearColor Brass(0.788f, 0.647f, 0.353f);      // #c9a55a
		static const FLinearColor BrassWarm(0.839f, 0.710f, 0.467f);  // #d6b577
		static const FLinearColor BrassDeep(0.541f, 0.435f, 0.200f);  // #8a6f33

		static const FLinearColor Oxblood(0.698f, 0.314f, 0.282f);    // #b25048
		static const FLinearColor OxbloodDeep(0.478f, 0.227f, 0.212f); // #7a3a36

		static const FLinearColor Verdigris(0.482f, 0.655f, 0.541f);  // #7ba78a
		static const FLinearColor Cobalt(0.490f, 0.639f, 0.776f);     // #7da3c6

		// Cores das nações (Belle Cartographie pastel)
		static const FLinearColor NationAlbion(0.651f, 0.765f, 0.839f);  // #a6c3d6
		static const FLinearColor NationGalia(0.839f, 0.659f, 0.659f);   // #d6a8a8
		static const FLinearColor NationNorden(0.839f, 0.753f, 0.557f);  // #d6c08e
	}

	// ── TEMA SÉPIA CARTOGRÁFICA ──────────────────────────────────────────────
	namespace Sepia
	{
		static const FLinearColor Bg0(0.769f, 0.706f, 0.541f);   // #c4b48a
		static const FLinearColor Bg1(0.878f, 0.816f, 0.659f);   // #e0d0a8
		static const FLinearColor Bg2(0.929f, 0.878f, 0.737f);   // #ece0bc
		static const FLinearColor Bg3(0.941f, 0.894f, 0.769f);   // #f0e4c4
		static const FLinearColor Bg4(0.961f, 0.918f, 0.800f);   // #f5eacc

		static const FLinearColor Text(0.165f, 0.125f, 0.078f);  // #2a2014
		static const FLinearColor Brass(0.478f, 0.310f, 0.110f); // #7a4f1c
		static const FLinearColor Oxblood(0.541f, 0.157f, 0.157f); // #8a2828
	}

	// ── TEMA OSSO DOCUMENTAL ─────────────────────────────────────────────────
	namespace Bone
	{
		static const FLinearColor Bg0(0.769f, 0.753f, 0.690f);   // #c4c0b0
		static const FLinearColor Bg1(0.831f, 0.816f, 0.753f);   // #d4d0c0
		static const FLinearColor Bg2(0.902f, 0.894f, 0.816f);   // #e6e4d0
		static const FLinearColor Bg3(0.929f, 0.918f, 0.816f);   // #ecead0
		static const FLinearColor Bg4(0.957f, 0.949f, 0.863f);   // #f4f2dc

		static const FLinearColor Text(0.122f, 0.110f, 0.078f);  // #1f1c14
		static const FLinearColor Brass(0.478f, 0.353f, 0.110f); // #7a5a1c
		static const FLinearColor Oxblood(0.549f, 0.196f, 0.165f); // #8c322a
	}
}
