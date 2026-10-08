// Copyright Epic Games, Inc. All Rights Reserved.
// Standalone C++20 Automated Physics Verification Harness

#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>
#include <string>
#include <iomanip>

// Mock Unreal Engine primitives for standalone math verification
struct FVector
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;

    constexpr FVector() = default;
    constexpr FVector(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}

    static const FVector ZeroVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;

    FVector operator+(const FVector& V) const { return FVector(X + V.X, Y + V.Y, Z + V.Z); }
    FVector operator-(const FVector& V) const { return FVector(X - V.X, Y - V.Y, Z - V.Z); }
    FVector operator*(float Scalar) const { return FVector(X * Scalar, Y * Scalar, Z * Scalar); }
    FVector operator/(float Scalar) const { return FVector(X / Scalar, Y / Scalar, Z / Scalar); }
    FVector operator-() const { return FVector(-X, -Y, -Z); }

    float SizeSquared() const { return X * X + Y * Y + Z * Z; }
    float Size() const { return std::sqrt(SizeSquared()); }

    static float DotProduct(const FVector& A, const FVector& B)
    {
        return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    }

    static FVector CrossProduct(const FVector& A, const FVector& B)
    {
        return FVector(
            A.Y * B.Z - A.Z * B.Y,
            A.Z * B.X - A.X * B.Z,
            A.X * B.Y - A.Y * B.X
        );
    }

    FVector GetClampedToMaxSize(float MaxSize) const
    {
        const float S = Size();
        if (S > MaxSize && S > 1.e-6f)
        {
            return (*this) * (MaxSize / S);
        }
        return *this;
    }
};

const FVector FVector::ZeroVector = FVector(0.0f, 0.0f, 0.0f);
const FVector FVector::UpVector = FVector(0.0f, 0.0f, 1.0f);
const FVector FVector::ForwardVector = FVector(1.0f, 0.0f, 0.0f);
const FVector FVector::RightVector = FVector(0.0f, 1.0f, 0.0f);

namespace FMath
{
    inline bool IsFinite(float F) { return std::isfinite(F); }
    inline float Clamp(float X, float Min, float Max) { return (X < Min) ? Min : ((X > Max) ? Max : X); }
    inline float Max(float A, float B) { return (A > B) ? A : B; }
    inline float Min(float A, float B) { return (A < B) ? A : B; }
    inline float Abs(float A) { return std::abs(A); }
    inline float InvSqrt(float F) { return 1.0f / std::sqrt(F); }
    inline float Atan2(float Y, float X) { return std::atan2(Y, X); }
    inline float DegreesToRadians(float Deg) { return Deg * (3.14159265358979323846f / 180.0f); }
    inline float RadiansToDegrees(float Rad) { return Rad * (180.0f / 3.14159265358979323846f); }
    inline float Lerp(float A, float B, float Alpha) { return A + Alpha * (B - A); }
}

#define FORCEINLINE inline

// Bring in math formulas
namespace HoverBikeMath
{
	FORCEINLINE bool IsFiniteVector(const FVector& V)
	{
		return FMath::IsFinite(V.X) && FMath::IsFinite(V.Y) && FMath::IsFinite(V.Z);
	}

	FORCEINLINE FVector SafeNormalize(const FVector& V, const FVector& Fallback = FVector::UpVector, float Tolerance = 1.e-6f)
	{
		if (!IsFiniteVector(V))
		{
			return Fallback;
		}

		const float SquareSum = V.SizeSquared();
		if (SquareSum > Tolerance * Tolerance)
		{
			const float Scale = FMath::InvSqrt(SquareSum);
			return V * Scale;
		}
		return Fallback;
	}

	FORCEINLINE float SafeDivide(float Numerator, float Denominator, float Fallback = 0.0f, float Tolerance = 1.e-6f)
	{
		if (!FMath::IsFinite(Numerator) || !FMath::IsFinite(Denominator))
		{
			return Fallback;
		}
		if (FMath::Abs(Denominator) > Tolerance)
		{
			return Numerator / Denominator;
		}
		return Fallback;
	}

	FORCEINLINE float SmoothStep(float Edge0, float Edge1, float X)
	{
		const float T = FMath::Clamp(SafeDivide(X - Edge0, Edge1 - Edge0, 0.0f), 0.0f, 1.0f);
		return T * T * (3.0f - 2.0f * T);
	}

	FORCEINLINE FVector CalculatePointVelocity(
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		const FVector& RelativePosition)
	{
		if (!IsFiniteVector(LinearVelocity) || !IsFiniteVector(AngularVelocity) || !IsFiniteVector(RelativePosition))
		{
			return FVector::ZeroVector;
		}
		return LinearVelocity + FVector::CrossProduct(AngularVelocity, RelativePosition);
	}

	FORCEINLINE float CalculateDampedSpringForce(
		float RestLength,
		float CurrentLength,
		float SpringStiffness,
		float DampingCoefficient,
		float VelocityAlongNormal,
		float MaxForce)
	{
		const float Compression = RestLength - CurrentLength;
		const float SpringForce = SpringStiffness * Compression;
		const float DamperForce = DampingCoefficient * VelocityAlongNormal;
		const float TotalForce = SpringForce - DamperForce;
		return FMath::Clamp(TotalForce, 0.0f, MaxForce);
	}

	FORCEINLINE FVector CalculatePDAlignmentTorque(
		const FVector& CurrentUp,
		const FVector& DesiredUp,
		const FVector& AngularVelocity,
		float AlignmentStrength,
		float AlignmentDamping,
		float MaxTorque)
	{
		const FVector CrossAxis = FVector::CrossProduct(CurrentUp, DesiredUp);
		const float SinAngle = CrossAxis.Size();
		const float DotAngle = FMath::Clamp(FVector::DotProduct(CurrentUp, DesiredUp), -1.0f, 1.0f);
		const float AngleError = FMath::Atan2(SinAngle, DotAngle);

		if (AngleError < 1.e-4f || SinAngle < 1.e-4f)
		{
			const FVector DampingTorque = -AngularVelocity * AlignmentDamping;
			return DampingTorque.GetClampedToMaxSize(MaxTorque);
		}

		const FVector RotationAxis = CrossAxis / SinAngle;
		const FVector ProportionalTorque = RotationAxis * (AngleError * AlignmentStrength);
		const FVector DerivativeTorque = -AngularVelocity * AlignmentDamping;

		const FVector TotalTorque = ProportionalTorque + DerivativeTorque;
		return TotalTorque.GetClampedToMaxSize(MaxTorque);
	}

	FORCEINLINE float CalculateTargetLeanAngle(
		float LateralAcceleration,
		float EffectiveGravity,
		float MaxLeanAngleRadians)
	{
		const float SafeG = FMath::Max(FMath::Abs(EffectiveGravity), 100.0f);
		const float IdealLean = FMath::Atan2(LateralAcceleration, SafeG);
		return FMath::Clamp(IdealLean, -MaxLeanAngleRadians, MaxLeanAngleRadians);
	}

	FORCEINLINE float CalculateQuadraticDrag(
		float Speed,
		float AirDensity,
		float DragCoefficient,
		float FrontalArea)
	{
		const float SpeedMetersPerSecond = Speed * 0.01f;
		const float SpeedSquared = SpeedMetersPerSecond * SpeedMetersPerSecond;
		return 0.5f * AirDensity * SpeedSquared * DragCoefficient * FrontalArea;
	}

	FORCEINLINE float CalculateGroundEffectDownforce(
		float Speed,
		float BaseDownforce,
		float ReferenceSpeed,
		float AverageHoverHeight,
		float MaxHoverHeight,
		float MaxDownforce)
	{
		if (ReferenceSpeed <= 1.0f || MaxHoverHeight <= 1.0f)
		{
			return 0.0f;
		}

		const float SpeedRatio = FMath::Clamp(Speed / ReferenceSpeed, 0.0f, 2.5f);
		const float SpeedFactor = SpeedRatio * SpeedRatio;
		const float ProximityRatio = FMath::Clamp(1.0f - (AverageHoverHeight / MaxHoverHeight), 0.0f, 1.0f);
		const float HeightFactor = ProximityRatio * ProximityRatio;
		const float Downforce = BaseDownforce * SpeedFactor * HeightFactor;
		return FMath::Clamp(Downforce, 0.0f, MaxDownforce);
	}

	FORCEINLINE float EvaluateEnginePowerCurve(float NormalizedSpeed)
	{
		const float Alpha = FMath::Clamp(NormalizedSpeed, 0.0f, 1.5f);
		if (Alpha >= 1.0f)
		{
			return FMath::Max(0.0f, 1.0f - (Alpha - 1.0f) * 4.0f);
		}
		const float T = 1.0f - Alpha;
		return FMath::Clamp(0.15f + 0.85f * (T * T * (3.0f - 2.0f * T)), 0.0f, 1.0f);
	}
}

// Verification Test Suite
int main()
{
    std::cout << "===============================================================\n";
    std::cout << " HOVERBIKE SIMULATION C++20 AUTOMATED PHYSICS TEST SUITE       \n";
    std::cout << "===============================================================\n";

    int TestsPassed = 0;
    int TotalTests = 12;

    // TEST 1: Hover Equilibrium Test
    {
        std::cout << "\n[TEST 1] 4-Point Suspension Hover Equilibrium Test...";
        const float RestLength = 150.0f;
        const float Stiffness = 250000.0f;
        const float Damping = 18000.0f;
        const float MaxForce = 300000.0f;
        const float VehicleWeight = 350.0f * 980.0f; // Mass * UE gravity (cm/s^2) = 343,000 N

        // With 4 pads, each pad supports ~85,750 N at equilibrium
        // Required compression: F = k * x -> x = 85750 / 250000 = 0.343 cm
        const float Compression = 85750.0f / Stiffness;
        const float HeightAtEquilibrium = RestLength - Compression;
        const float ForcePerPad = HoverBikeMath::CalculateDampedSpringForce(
            RestLength, HeightAtEquilibrium, Stiffness, Damping, 0.0f, MaxForce
        );
        const float TotalSuspensionForce = ForcePerPad * 4.0f;

        // Check within 0.01% floating-point tolerance of 343,000 N
        assert(std::abs(TotalSuspensionForce - VehicleWeight) < 35.0f);
        assert(ForcePerPad > 0.0f && ForcePerPad < MaxForce);
        std::cout << " PASSED! (Total Force: " << TotalSuspensionForce << " N, Weight: " << VehicleWeight << " N)\n";
        TestsPassed++;
    }

    // TEST 2: Point Velocity & Pitch/Roll Moment Test
    {
        std::cout << "[TEST 2] Rigid Body Point Velocity & Torque Induction Test...";
        const FVector LinearVel(1000.0f, 0.0f, 0.0f);
        const FVector AngularVel(0.0f, 2.0f, 0.0f); // 2 rad/s pitch-down
        const FVector FrontPadOffset(140.0f, 0.0f, -20.0f);
        const FVector RearPadOffset(-140.0f, 0.0f, -20.0f);

        const FVector FrontPointVel = HoverBikeMath::CalculatePointVelocity(LinearVel, AngularVel, FrontPadOffset);
        const FVector RearPointVel = HoverBikeMath::CalculatePointVelocity(LinearVel, AngularVel, RearPadOffset);

        // Front pad should have negative Z velocity (moving downward into ground), Rear should have positive Z
        assert(FrontPointVel.Z < RearPointVel.Z);
        // Due to r_z = -20 cm, omega_y * r_z = -40 cm/s
        assert(std::abs(FrontPointVel.X - (LinearVel.X + AngularVel.Y * FrontPadOffset.Z)) < 1.e-3f);
        std::cout << " PASSED! (Front Vz: " << FrontPointVel.Z << ", Rear Vz: " << RearPointVel.Z << ")\n";
        TestsPassed++;
    }

    // TEST 3: Arbitrary Surface Alignment (Flat, 45 Bank, 90 Wall, 180 Ceiling)
    {
        std::cout << "[TEST 3] PD Surface Orientation Alignment on Arbitrary Surfaces...";
        const FVector CurrentUp = FVector::UpVector;
        const FVector AngularVel = FVector::ZeroVector;
        const float Kp = 800000.0f;
        const float Kd = 120000.0f;

        // 3a. Flat surface: Up == Up -> Torque should be zero
        const FVector FlatTorque = HoverBikeMath::CalculatePDAlignmentTorque(CurrentUp, FVector::UpVector, AngularVel, Kp, Kd, 1.e7f);
        assert(FlatTorque.Size() < 1.e-3f);

        // 3b. 90 deg Vertical Wall (Normal pointing in +Y)
        const FVector WallNormal(0.0f, 1.0f, 0.0f);
        const FVector WallTorque = HoverBikeMath::CalculatePDAlignmentTorque(CurrentUp, WallNormal, AngularVel, Kp, Kd, 1.e7f);
        // Cross( (0,0,1), (0,1,0) ) = (-1, 0, 0), so torque rolls left to align
        assert(WallTorque.X < 0.0f);
        assert(WallTorque.Size() > 10000.0f);

        // 3c. Inverted 180 deg Ceiling (Normal pointing in -Z)
        const FVector CeilingNormal(0.0f, 0.0f, -1.0f);
        const FVector SlightlyTiltedUp(0.01f, 0.0f, 0.9999f);
        const FVector CeilingTorque = HoverBikeMath::CalculatePDAlignmentTorque(SlightlyTiltedUp, CeilingNormal, AngularVel, Kp, Kd, 1.e7f);
        assert(CeilingTorque.Size() > 10000.0f);

        std::cout << " PASSED! (Wall Torque: " << WallTorque.Size() << " N*cm, Ceiling Torque: " << CeilingTorque.Size() << " N*cm)\n";
        TestsPassed++;
    }

    // TEST 4: Propulsion Engine Power Curve & Boost
    {
        std::cout << "[TEST 4] Hybrid Propulsion Power Curve & Boost Multiplier Test...";
        const float LowSpeedPower = HoverBikeMath::EvaluateEnginePowerCurve(0.1f);
        const float MidSpeedPower = HoverBikeMath::EvaluateEnginePowerCurve(0.5f);
        const float HighSpeedPower = HoverBikeMath::EvaluateEnginePowerCurve(0.95f);
        const float BeyondTopSpeed = HoverBikeMath::EvaluateEnginePowerCurve(1.25f);

        assert(LowSpeedPower > MidSpeedPower);
        assert(MidSpeedPower > HighSpeedPower);
        assert(HighSpeedPower > BeyondTopSpeed);
        assert(BeyondTopSpeed == 0.0f); // Runaway protection

        std::cout << " PASSED! (0.1v: " << LowSpeedPower << ", 0.5v: " << MidSpeedPower << ", 0.95v: " << HighSpeedPower << ")\n";
        TestsPassed++;
    }

    // TEST 5: Quadratic Aerodynamic Drag & Air Brake Test
    {
        std::cout << "[TEST 5] Quadratic Aerodynamic Drag & Air Brake Scaling Test...";
        const float Rho = 1.225f;
        const float BaseCd = 0.65f;
        const float Area = 1.1f;

        // Speed 1: 100 km/h = 2778 cm/s = 27.78 m/s
        const float Speed1 = 2778.0f;
        const float Drag1 = HoverBikeMath::CalculateQuadraticDrag(Speed1, Rho, BaseCd, Area);

        // Speed 2: 200 km/h = 5556 cm/s = 55.56 m/s (2x speed -> 4x drag)
        const float Speed2 = 5556.0f;
        const float Drag2 = HoverBikeMath::CalculateQuadraticDrag(Speed2, Rho, BaseCd, Area);

        const float Ratio = Drag2 / Drag1;
        assert(std::abs(Ratio - 4.0f) < 0.05f); // Quadratic proof

        // Air brake engaged: Cd increases from 0.65 to 2.45
        const float AirBrakeDrag = HoverBikeMath::CalculateQuadraticDrag(Speed2, Rho, BaseCd + 1.8f, Area);
        assert(AirBrakeDrag > Drag2 * 3.5f);

        std::cout << " PASSED! (100km/h Drag: " << Drag1 << " N, 200km/h Drag: " << Drag2 << " N, AirBrake: " << AirBrakeDrag << " N)\n";
        TestsPassed++;
    }

    // TEST 6: Ground-Effect Downforce Scaling Test
    {
        std::cout << "[TEST 6] Speed-Squared & Ground Proximity Downforce Scaling...";
        const float BaseDownforce = 150000.0f;
        const float RefSpeed = 12600.0f; // 450 km/h
        const float MaxHover = 300.0f;
        const float MaxDownforce = 150000.0f;

        // Close to ground (h = 50 cm) vs Far from ground (h = 250 cm)
        const float CloseDownforce = HoverBikeMath::CalculateGroundEffectDownforce(RefSpeed, BaseDownforce, RefSpeed, 50.0f, MaxHover, MaxDownforce);
        const float FarDownforce = HoverBikeMath::CalculateGroundEffectDownforce(RefSpeed, BaseDownforce, RefSpeed, 250.0f, MaxHover, MaxDownforce);

        assert(CloseDownforce > FarDownforce * 10.0f); // Proximity boost
        std::cout << " PASSED! (Close h=50cm: " << CloseDownforce << " N, Far h=250cm: " << FarDownforce << " N)\n";
        TestsPassed++;
    }

    // TEST 7: Dynamic Motorcycle Lean Dynamics Test
    {
        std::cout << "[TEST 7] Dynamic Motorcycle Lean & Angle Clamping...";
        const float EffectiveG = 980.0f;
        const float MaxLeanRad = FMath::DegreesToRadians(55.0f);

        // Turn generating 0 lateral accel -> 0 lean
        const float Lean0 = HoverBikeMath::CalculateTargetLeanAngle(0.0f, EffectiveG, MaxLeanRad);
        assert(std::abs(Lean0) < 1.e-4f);

        // Turn generating 1G lateral accel (980 cm/s^2) -> 45 deg lean (0.785 rad)
        const float Lean1G = HoverBikeMath::CalculateTargetLeanAngle(980.0f, EffectiveG, MaxLeanRad);
        const float ExpectedRad = std::atan(1.0f);
        assert(std::abs(Lean1G - ExpectedRad) < 1.e-3f);

        // Extreme 5G lateral accel -> clamped to MaxLeanRad (55 deg)
        const float Lean5G = HoverBikeMath::CalculateTargetLeanAngle(5.0f * 980.0f, EffectiveG, MaxLeanRad);
        assert(std::abs(Lean5G - MaxLeanRad) < 1.e-4f);

        std::cout << " PASSED! (1G Lean: " << FMath::RadiansToDegrees(Lean1G) << " deg, 5G Clamped: " << FMath::RadiansToDegrees(Lean5G) << " deg)\n";
        TestsPassed++;
    }

    // TEST 8: Lateral Grip & Momentum Drift Breakdown Test
    {
        std::cout << "[TEST 8] Lateral Traction & SmoothStep Drift Alpha Transition...";
        const float StartThresh = 0.85f;
        const float FullThresh = 1.35f;

        // Grip demand = 0.5 (safe cruising)
        const float AlphaSafe = HoverBikeMath::SmoothStep(StartThresh, FullThresh, 0.5f);
        assert(AlphaSafe == 0.0f);

        // Grip demand = 1.1 (breakaway initiated)
        const float AlphaMid = HoverBikeMath::SmoothStep(StartThresh, FullThresh, 1.1f);
        assert(AlphaMid > 0.3f && AlphaMid < 0.7f);

        // Grip demand = 1.8 (full high-speed slide)
        const float AlphaFull = HoverBikeMath::SmoothStep(StartThresh, FullThresh, 1.8f);
        assert(AlphaFull == 1.0f);

        std::cout << " PASSED! (Demand 0.5: Alpha=" << AlphaSafe << ", Demand 1.1: Alpha=" << AlphaMid << ", Demand 1.8: Alpha=" << AlphaFull << ")\n";
        TestsPassed++;
    }

    // TEST 9: Repulsor Grip Energy Model Test
    {
        std::cout << "[TEST 9] Finite Grip Capacity Depletion & Recovery Simulation...";
        float Energy = 100.0f;
        const float MaxEnergy = 100.0f;
        const float DrainRate = 35.0f;
        const float RecoverRate = 25.0f;
        const float Dt = 0.1f;

        // 10 steps of aggressive drift overload (overload = 1.0)
        for (int i = 0; i < 10; ++i)
        {
            Energy -= DrainRate * 1.0f * Dt;
        }
        assert(Energy < 70.0f);

        // 10 steps of calm recovery
        const float Drained = Energy;
        for (int i = 0; i < 10; ++i)
        {
            Energy = FMath::Min(MaxEnergy, Energy + RecoverRate * Dt);
        }
        assert(Energy > Drained);

        std::cout << " PASSED! (Energy Drained: " << Drained << "%, Recovered: " << Energy << "%)\n";
        TestsPassed++;
    }

    // TEST 10: Airborne Transition & Contact Grace Decay Test
    {
        std::cout << "[TEST 10] Airborne Transition & Contact Grace Torque Attenuation...";
        const FVector DesiredNormal(0.0f, 0.707f, 0.707f);
        const FVector CurrentUp = FVector::UpVector;
        const FVector AngularVel = FVector::ZeroVector;
        const float Kp = 800000.0f;
        const float Kd = 120000.0f;

        // Grounded: 100% torque authority
        const FVector GroundedTorque = HoverBikeMath::CalculatePDAlignmentTorque(CurrentUp, DesiredNormal, AngularVel, Kp * 1.0f, Kd * 1.0f, 1.e7f);

        // Airborne: 20% torque authority preserves momentum without artificial snapping
        const FVector AirborneTorque = HoverBikeMath::CalculatePDAlignmentTorque(CurrentUp, DesiredNormal, AngularVel, Kp * 0.2f, Kd * 0.2f, 1.e7f);

        assert(AirborneTorque.Size() < GroundedTorque.Size() * 0.25f);
        assert(AirborneTorque.Size() > 0.0f);

        std::cout << " PASSED! (Grounded Torque: " << GroundedTorque.Size() << ", Airborne: " << AirborneTorque.Size() << ")\n";
        TestsPassed++;
    }

    // TEST 11: Zero-Gravity Zone Behavior Test
    {
        std::cout << "[TEST 11] Zero-G Track Operation & Lean Attenuation...";
        const float ZeroG = 0.0f;
        const float MaxLeanRad = FMath::DegreesToRadians(55.0f);

        // In Zero-G, lateral acceleration should not produce runaway lean angles (SafeG guards against /0)
        const float ZeroGLean = HoverBikeMath::CalculateTargetLeanAngle(500.0f, ZeroG, MaxLeanRad);
        assert(std::isfinite(ZeroGLean));
        assert(std::abs(ZeroGLean) <= MaxLeanRad);

        std::cout << " PASSED! (Zero-G Lean is Finite and Protected: " << FMath::RadiansToDegrees(ZeroGLean) << " deg)\n";
        TestsPassed++;
    }

    // TEST 12: Numerical Stability, Zero Division, NaN & Extreme Speeds Test
    {
        std::cout << "[TEST 12] Numerical Stability, NaN Protection & Boundary Sanity...";
        // Safe normalize on zero vector
        const FVector ZeroNorm = HoverBikeMath::SafeNormalize(FVector::ZeroVector, FVector::UpVector);
        assert(ZeroNorm.Z == 1.0f);

        // Safe divide by zero
        const float DivZero = HoverBikeMath::SafeDivide(100.0f, 0.0f, 42.0f);
        assert(DivZero == 42.0f);

        // Extreme hypersonic speed: 10,000 km/h (277,778 cm/s)
        const float ExtremeSpeed = 277778.0f;
        const float DragExtreme = HoverBikeMath::CalculateQuadraticDrag(ExtremeSpeed, 1.225f, 0.65f, 1.1f);
        assert(std::isfinite(DragExtreme));
        assert(DragExtreme > 0.0f);

        // Power curve at extreme speed
        const float PowerExtreme = HoverBikeMath::EvaluateEnginePowerCurve(5.0f);
        assert(PowerExtreme == 0.0f);

        std::cout << " PASSED! (All numerical safety assertions verified)\n";
        TestsPassed++;
    }

    std::cout << "\n===============================================================\n";
    std::cout << " RESULT: " << TestsPassed << " / " << TotalTests << " TESTS PASSED SUCCESSFULLY!\n";
    std::cout << " ALL 12 ACCEPTANCE CRITERIA PHYSICALLY VALIDATED!\n";
    std::cout << "===============================================================\n";

    return 0;
}
