/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MoveSpline.h"

#include <sstream>

#include "Logging/Logger.hpp"

namespace MovementMgr {

#if VERSION_STRING >= Cata
Location MoveSpline::ComputePosition() const
{
    ASSERT(Initialized());

    float u = 1.f;
    int32_t seg_time = spline.length(point_Idx, point_Idx + 1);
    if (seg_time > 0)
        u = (time_passed - spline.length(point_Idx)) / (float)seg_time;
    Location c;
    c.orientation = initialOrientation;
    spline.evaluate_percent(point_Idx, u, c);

    if (splineflags.animation)
        ;// MoveSplineFlag::Animation disables falling or parabolic movement
    else if (splineflags.parabolic)
        computeParabolicElevation(c.z);
    else if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags.done && splineflags.isFacing())
    {
        if (splineflags.final_angle)
            c.orientation = facing.angle;
        else if (splineflags.final_point)
            c.orientation = std::atan2(facing.f.y - c.y, facing.f.x - c.x);
        //nothing to do for MoveSplineFlag::Final_Target flag
    }
    else
    {
        if (!splineflags.hasFlag(MoveSplineFlag::OrientationFixed | MoveSplineFlag::Falling))
        {
            Vector3 hermite;
            spline.evaluate_derivative(point_Idx, u, hermite);
            c.orientation = std::atan2(hermite.y, hermite.x);
        }

        if (splineflags.orientationInversed)
            c.orientation = -c.orientation;
    }
    return c;
}
#elif VERSION_STRING == WotLK
Location MoveSpline::ComputePosition() const
{
    ASSERT(Initialized());

    float u = 1.f;
    int32_t seg_time = spline.length(point_Idx, point_Idx + 1);
    if (seg_time > 0)
        u = (time_passed - spline.length(point_Idx)) / (float)seg_time;
    Location c;
    c.orientation = initialOrientation;
    spline.evaluate_percent(point_Idx, u, c);

    if (splineflags.animation)
        ;// MoveSplineFlag::Animation disables falling or parabolic movement
    else if (splineflags.parabolic)
        computeParabolicElevation(c.z);
    else if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags.done && splineflags.isFacing())
    {
        if (splineflags.final_angle)
            c.orientation = facing.angle;
        else if (splineflags.final_point)
            c.orientation = std::atan2(facing.f.y - c.y, facing.f.x - c.x);
        //nothing to do for MoveSplineFlag::Final_Target flag
    }
    else
    {
        if (!splineflags.hasFlag(MoveSplineFlag::OrientationFixed | MoveSplineFlag::Falling))
        {
            Vector3 hermite;
            spline.evaluate_derivative(point_Idx, u, hermite);
            c.orientation = std::atan2(hermite.y, hermite.x);
        }

        if (splineflags.backward)
            c.orientation = c.orientation - float(M_PI);
    }
    return c;
}
#else
Location MoveSpline::ComputePosition() const
{
    ASSERT(Initialized());

    float u = 1.f;
    int32_t seg_time = spline.length(point_Idx, point_Idx + 1);
    if (seg_time > 0)
        u = (time_passed - spline.length(point_Idx)) / (float)seg_time;
    Location c;
    c.orientation = initialOrientation;
    spline.evaluate_percent(point_Idx, u, c);
    
    if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags.done && splineflags.isFacing())
    {
        if (splineflags.final_angle)
            c.orientation = facing.angle;
        else if (splineflags.final_point)
            c.orientation = std::atan2(facing.f.y - c.y, facing.f.x - c.x);
        //nothing to do for MoveSplineFlag::Final_Target flag
    }
    else
    {
        if (!splineflags.hasFlag(MoveSplineFlag::Falling))
        {
            Vector3 hermite;
            spline.evaluate_derivative(point_Idx, u, hermite);
            c.orientation = std::atan2(hermite.y, hermite.x);
        }
    }
    return c;
}
#endif

void MoveSpline::computeParabolicElevation(float& el) const
{
    if (time_passed > effect_start_time)
    {
        float t_passedf = MSToSec(time_passed - effect_start_time);
        float t_durationf = MSToSec(Duration() - effect_start_time); // client use not modified duration here

        // -a*x*x + bx + c:
        //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
        el += (t_durationf - t_passedf) * 0.5f * vertical_acceleration * t_passedf;
    }
}

void MoveSpline::computeFallElevation(float& el) const
{
    float z_now = spline.getPoint(spline.first()).z - MovementMgr::computeFallElevation(MSToSec(time_passed), false);
    float final_z = FinalDestination().z;
    el = std::max(z_now, final_z);
}

inline uint32_t computeDuration(float length, float velocity)
{
    return SecToMS(length / velocity);
}

struct FallInitializer
{
    FallInitializer(float _start_elevation) : start_elevation(_start_elevation) { }
    float start_elevation;
    inline int32_t operator()(Spline<int32>& s, int32_t i)
    {
        return static_cast<int32_t>(MovementMgr::computeFallTime(start_elevation - s.getPoint(i + 1).z, false) * 1000.f);
    }
};

enum
{
    minimal_duration = 1
};

struct CommonInitializer
{
    CommonInitializer(float _velocity) : velocityInv(1000.f/_velocity), time(minimal_duration) { }
    float velocityInv;
    int32_t time;
    inline int32_t operator()(Spline<int32>& s, int32_t i)
    {
        time += static_cast<int32_t>(s.SegLength(i) * velocityInv);
        return time;
    }
};

void MoveSpline::init_spline(MoveSplineInitArgs const& args)
{
    static SplineBase::EvaluationMode const modes[2] = { SplineBase::ModeLinear, SplineBase::ModeCatmullrom };
    if (args.flags.cyclic)
    {
        uint32_t cyclic_point = 0;
        if (splineflags.enter_cycle)
            cyclic_point = 1; // shouldn't be modified, came from client
        spline.init_cyclic_spline(&args.path[0], static_cast<int32_t>(args.path.size()), modes[args.flags.isSmooth()], cyclic_point, args.initialOrientation);
    }
    else
    {
        spline.init_spline(&args.path[0], static_cast<int32_t>(args.path.size()), modes[args.flags.isSmooth()], args.initialOrientation);
    }

    // init spline timestamps
    if (splineflags.falling)
    {
        FallInitializer init(spline.getPoint(spline.first()).z);
        spline.initLengths(init);
    }
    else
    {
        CommonInitializer init(args.velocity);
        spline.initLengths(init);
    }

    /// @todo what to do in such cases? problem is in input data (all points are at same coords)
    if (spline.length() < minimal_duration)
    {
        sLogger.failure("MoveSpline::init_spline: zero length spline, wrong input data?");
        spline.set_length(spline.last(), spline.isCyclic() ? 1000 : 1);
    }
    point_Idx = spline.first();
}

void MoveSpline::Initialize(MoveSplineInitArgs const& args)
{
    splineflags = args.flags;
    facing = args.facing;
    m_Id = args.splineId;
    point_Idx_offset = args.path_Idx_offset;
    initialOrientation = args.initialOrientation;

    time_passed = 0;
    vertical_acceleration = 0.f;
    effect_start_time = 0;

    velocity = args.velocity;

    // Check if its a stop spline
    if (args.flags.done)
    {
        spline.clear();
        return;
    }

    init_spline(args);
#if VERSION_STRING > TBC
    // init parabolic / animation
    // spline initialized, duration known and i able to compute parabolic acceleration
    if (args.flags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
    {
        effect_start_time = static_cast<int32_t>(Duration() * args.time_perc);
        if (args.flags.parabolic && effect_start_time < Duration())
        {
            float f_duration = MSToSec(Duration() - effect_start_time);
            vertical_acceleration = args.parabolic_amplitude * 8.f / (f_duration * f_duration);
        }
    }
#endif
}

MoveSpline::MoveSpline() : m_Id(0), time_passed(0),
    vertical_acceleration(0.f), initialOrientation(0.f), effect_start_time(0), point_Idx(0), point_Idx_offset(0), velocity(0.f),
    onTransport(false)
{
    splineflags.done = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool MoveSplineInitArgs::Validate(Unit* unit) const
{
#define CHECK(exp) \
    if (!(exp))\
    {\
        if (unit)\
            sLogger.failure("misc.movesplineinitargs MoveSplineInitArgs::Validate: expression '{}' failed", #exp);\
        else\
            sLogger.failure("misc.movesplineinitargs MoveSplineInitArgs::Validate: expression '{}' failed for cyclic spline continuation", #exp); \
        return false;\
    }
    CHECK(path.size() > 1);
    CHECK(velocity >= 0.001f);
    CHECK(time_perc >= 0.f && time_perc <= 1.f);
    //CHECK(_checkPathBounds());
    return true;
#undef CHECK
}

// MONSTER_MOVE packet format limitation for not CatmullRom movement:
// each vertex offset packed into 11 bytes
bool MoveSplineInitArgs::_checkPathBounds() const
{
    if (!(flags & MoveSplineFlag::Mask_CatmullRom) && path.size() > 2)
    {
        enum{
            MAX_OFFSET = (1 << 11) / 2
        };
        Vector3 middle = (path.front()+path.back()) / 2;
        Vector3 offset;
        for (uint32_t i = 1; i < path.size()-1; ++i)
        {
            offset = path[i] - middle;
            if (std::fabs(offset.x) >= static_cast<float>(MAX_OFFSET) || std::fabs(offset.y) >= static_cast<float>(MAX_OFFSET) || std::fabs(offset.z) >= static_cast<float>(MAX_OFFSET))
            {
                sLogger.failure("MoveSplineInitArgs::_checkPathBounds check failed");
                return false;
            }
        }
    }
    return true;
}

MoveSplineInitArgs::MoveSplineInitArgs(size_t path_capacity /*= 16*/) : path_Idx_offset(0), velocity(0.f),
parabolic_amplitude(0.f), time_perc(0.f), splineId(0), initialOrientation(0.f),
walk(false), HasVelocity(false), TransformForTransport(true)
{
    path.reserve(path_capacity);
}

MoveSplineInitArgs::MoveSplineInitArgs(MoveSplineInitArgs && args) = default;
MoveSplineInitArgs::~MoveSplineInitArgs() = default;

//////////////////////////////////////////////////////////////////////////////////////////

MoveSpline::UpdateResult MoveSpline::_updateState(int32_t& ms_time_diff)
{
    if (Finalized())
    {
        ms_time_diff = 0;
        return Result_Arrived;
    }

    UpdateResult result = Result_None;

    int32_t minimal_diff = std::min(ms_time_diff, segment_time_elapsed());
    ASSERT(minimal_diff >= 0);
    time_passed += minimal_diff;
    ms_time_diff -= minimal_diff;

    if (time_passed >= next_timestamp())
    {
        ++point_Idx;
        if (point_Idx < spline.last())
        {
            result = Result_NextSegment;
        }
        else
        {
            if (spline.isCyclic())
            {
                point_Idx = spline.first();
                time_passed = time_passed % Duration();
                result = Result_NextCycle;

                // Remove first point from the path after one full cycle.
                // That point was the position of the unit prior to entering the cycle and it shouldn't be repeated with continuous cycles.
                if (splineflags.enter_cycle)
                {
                    splineflags.enter_cycle = false;

                    MoveSplineInitArgs args{spline.getPointCount()};
                    args.path.assign(spline.getPoints().begin() + spline.first() + 1, spline.getPoints().begin() + spline.last());
                    args.facing = facing;
                    args.flags = splineflags;
                    args.path_Idx_offset = point_Idx_offset;
                    // MoveSplineFlag::Parabolic | MoveSplineFlag::Animation not supported currently
                    //args.parabolic_amplitude = ?;
                    //args.time_perc = ?;
                    args.splineId = m_Id;
                    args.initialOrientation = initialOrientation;
                    args.velocity = 1.0f; // Calculated below
                    args.HasVelocity = true;
                    args.TransformForTransport = onTransport;
                    if (args.Validate(nullptr))
                    {
                        // New cycle should preserve previous cycle's duration for some weird reason, even though
                        // the path is really different now. Blizzard is weird. Or this was just a simple oversight.
                        // Since our splines precalculate length with velocity in mind, if we want to find the desired
                        // velocity, we have to make a fake spline, calculate its duration and then compare it to the
                        // desired duration, thus finding out how much the velocity has to be increased for them to match.
                        MoveSpline tempSpline;
                        tempSpline.Initialize(args);
                        args.velocity = (float)tempSpline.Duration() / Duration();

                        if (args.Validate(nullptr))
                            init_spline(args);
                    }
                }
            }
            else
            {
                _Finalize();
                ms_time_diff = 0;
                result = Result_Arrived;
            }
        }
    }

    return result;
}

std::string MoveSpline::ToString() const
{
    std::stringstream str;
    str << "MoveSpline" << "\n";
    str << "spline Id: " << GetId() << "\n";
    str << "flags: " << splineflags.ToString() << "\n";
    if (splineflags.final_angle)
        str << "facing  angle: " << facing.angle;
    else if (splineflags.final_target)
        str << "facing target: " << facing.target;
    else if (splineflags.final_point)
        str << "facing  point: " << facing.f.x << " " << facing.f.y << " " << facing.f.z;
    str << "\n";
    str << "time passed: " << time_passed << "\n";
    str << "total  time: " << Duration() << "\n";
    str << "spline point Id: " << point_Idx << "\n";
    str << "path  point  Id: " << currentPathIdx() << "\n";
    str << spline.ToString();
    return str.str();
}

void MoveSpline::_Finalize()
{
    splineflags.done = true;
    point_Idx = spline.last() - 1;
    time_passed = Duration();
}

int32_t MoveSpline::currentPathIdx() const
{
    int32_t point = point_Idx_offset + point_Idx - spline.first() + (int)Finalized();
    if (isCyclic())
        point = point % (spline.last()-spline.first());
    return point;
}
} // namespace MovementMgr
