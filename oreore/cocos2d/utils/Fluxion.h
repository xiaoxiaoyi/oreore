#ifndef __OREORE_COCOS2D_FLUXION_H__
#define __OREORE_COCOS2D_FLUXION_H__

/*
 * Fluxion -FLUently aCTION Generator-
 */

#include <type_traits>
#include <limits>
#include "cocos2d.h"

namespace oreore
{
    namespace Fluxion
    {
        class SequentialAction;
        class ParallelAction;
        class Repeat;
        class Ease;
        
        struct Action
        {
            inline SequentialAction operator>>(const Action &action);
            inline SequentialAction operator>>(cocos2d::Action *action);
            inline SequentialAction operator>>(const std::function<void()> &action);
            virtual operator cocos2d::Action *() const = 0;
        };

        class ActionInterval : public Action
        {
        private:
            template<typename T>
            inline T __mul(const T &op, typename std::enable_if<std::is_base_of<Ease, T>::value>::type* = nullptr)
            {
                return op.apply(this);
            }

            template<typename T>
            inline Repeat __mul(const T &op, typename std::enable_if<std::numeric_limits<T>::is_integer>::type* = nullptr);
        public:
            inline ParallelAction operator+(const ActionInterval &action);
            inline ParallelAction operator+(cocos2d::ActionInterval *action);
            template<typename T>
            inline auto operator*(const T &op) -> decltype(__mul(op)) { return __mul(op); }
            virtual operator cocos2d::ActionInterval *() const = 0;
        };
        
        class Ease : public ActionInterval
        {
        private:
        public:
            Ease() { }
            virtual ~Ease() { }
        };

        template<typename T>
        class WrapAction : public Action
        {
        private:
            T *action;
        public:
            template<typename... Args>
            inline WrapAction(const Args... args) : action(T::create(args...)) { }
            inline operator cocos2d::Action *() const override { return action; }
        };

        template<typename T>
        class WrapFiniteAction : public ActionInterval
        {
        private:
            T *action;
        public:
            template<typename... Args>
            inline WrapFiniteAction(const Args... args) : action(T::create(args...)) { }
            inline operator cocos2d::ActionInterval *() const override { return action; }
            inline operator cocos2d::Action *() const override { return action; }
        };

        class SequentialAction : public ActionInterval
        {
        private:
            std::unique_ptr<cocos2d::Array> actions;
        public:
            inline SequentialAction() : actions(new cocos2d::Array())
            {
                actions->initWithCapacity(4);
            }

            template<typename S, typename T>
            inline SequentialAction(const S &a, const T &b) : actions(new cocos2d::Array())
            {
                actions->initWithObject(static_cast<cocos2d::Action *>(a));
                actions->addObject(static_cast<cocos2d::Action *>(b));
            }

            inline SequentialAction(cocos2d::Array *array) : actions(array) { }

            inline SequentialAction &operator>>(const Action &action)
            {
                actions->addObject(static_cast<cocos2d::Action *>(action));
                return *this;
            }

            inline SequentialAction &operator>>(cocos2d::Action *action)
            {
                actions->addObject(action);
                return *this;
            }

            inline SequentialAction &operator>>(const std::function<void()> &func)
            {
                actions->addObject(cocos2d::CallFunc::create(func));
                return *this;
            }

            inline operator cocos2d::Action *() const override
            {
                return cocos2d::Sequence::create(actions.get());
            }

            inline operator cocos2d::ActionInterval *() const override
            {
                return cocos2d::Sequence::create(actions.get());
            }

            template<typename... Args>
            inline static SequentialAction createFromActions(const Args &...args)
            {
                return SequentialAction(cocos2d::Array::create(static_cast<cocos2d::Action *>(args)..., nullptr));
            }
        };

        class ParallelAction : public ActionInterval
        {
            friend class ActionInterval;
        private:
            std::unique_ptr<cocos2d::Array> actions;
        public:
            inline ParallelAction() : actions(new cocos2d::Array())
            {
                actions->initWithCapacity(2);
            }
            
            inline ParallelAction(cocos2d::Array *array) : actions(array) { }

            template<typename S, typename T>
            inline ParallelAction(const S &a, const T &b) : actions(new cocos2d::Array())
            {
                actions->initWithObject(static_cast<cocos2d::ActionInterval *>(a));
                actions->addObject(static_cast<cocos2d::ActionInterval *>(b));
            }

            inline ParallelAction &operator+(const ActionInterval &action)
            {
                actions->addObject(static_cast<cocos2d::ActionInterval *>(action));
                return *this;
            }

            inline ParallelAction &operator+(cocos2d::ActionInterval *action)
            {
                actions->addObject(action);
                return *this;
            }

            inline operator cocos2d::Action *() const override
            {
                return cocos2d::Spawn::create(actions.get());
            }

            inline operator cocos2d::ActionInterval *() const override
            {
                return cocos2d::Spawn::create(actions.get());
            }

            template<typename... Args>
            inline static ParallelAction createFromActions(const Args &...args)
            {
                return ParallelAction(cocos2d::Array::create(static_cast<cocos2d::ActionInterval *>(args)..., nullptr));
            }
        };

        class Repeat : public ActionInterval
        {
            cocos2d::Repeat *action;
        private:
        public:
            Repeat(const ActionInterval &action, const int times) : action(cocos2d::Repeat::create(action, times)) { }
            inline operator cocos2d::Action *() const override { return action; }
            inline operator cocos2d::ActionInterval *() const override { return action; }
        };

        template<typename T>
        class WrapEase : public Ease
        {
        private:
            T *ease;
        public:
            WrapEase() : ease(nullptr) { }
            WrapEase(T *ease) : ease(ease) { }
            ~WrapEase() { }

            inline operator cocos2d::Action *() const { return ease; }
            inline operator cocos2d::ActionInterval *() const { return ease; }
            inline WrapEase<T> apply(const ActionInterval *action) const
            {
                return WrapEase<T>(T::create(static_cast<cocos2d::ActionInterval *>(*action)));
            }
        };

        template<typename T, typename P>
        class WrapEase1 : public Ease
        {
        private:
            T *ease;
            P param;
        public:
            WrapEase1() : ease(nullptr), param() { }
            WrapEase1(T *ease) : ease(ease), param() { }
            WrapEase1(const P param) : ease(ease), param(param) { }
            WrapEase1(T *ease, const P param) : ease(ease), param(param) { }
            ~WrapEase1() { }

            inline operator cocos2d::Action *() const { return ease; }
            inline operator cocos2d::ActionInterval *() const { return ease; }
            inline WrapEase1<T, P> apply(const ActionInterval *action) const
            {
                return WrapEase1<T, P>(T::create(static_cast<cocos2d::ActionInterval *>(*action), param), param);
            }
        };

        /* Action impl */
        inline SequentialAction Action::operator>>(const Action &action)
        {
            return SequentialAction(*this, action);
        }
    
        inline SequentialAction Action::operator>>(cocos2d::Action *action)
        {
            return SequentialAction(*this, action);
        }

        inline SequentialAction Action::operator>>(const std::function<void()> &action)
        {
            return SequentialAction(*this, cocos2d::CallFunc::create(action));
        }
        
        /* ActionInterval impl */
        inline ParallelAction ActionInterval::operator+(const ActionInterval &action)
        {
            return ParallelAction(*this, action);
        }
    
        inline ParallelAction ActionInterval::operator+(cocos2d::ActionInterval *action)
        {
            return ParallelAction(*this, action);
        }

        template<typename T>
        inline Repeat ActionInterval::__mul(const T &op, typename std::enable_if<std::numeric_limits<T>::is_integer>::type*)
        {
            return Repeat(*this, op);
        }
    }

    inline Fluxion::SequentialAction operator>>(cocos2d::Action *a, const Fluxion::Action &b)
    {
        return Fluxion::SequentialAction(a, b);
    }

    inline Fluxion::ParallelAction operator+(cocos2d::ActionInterval *a, const Fluxion::ActionInterval &b)
    {
        return Fluxion::ParallelAction(a, static_cast<cocos2d::ActionInterval *>(b));
    }

    namespace x
    {
        /* finite time action */
        typedef Fluxion::WrapFiniteAction<cocos2d::MoveTo> MoveTo;
        inline MoveTo moveTo(const float duration, const cocos2d::Point &pos) { return MoveTo(duration, pos); }
        inline MoveTo moveTo(const float duration, const float x, const float y) { return moveTo(duration, cocos2d::Point(x, y)); }
        
        typedef Fluxion::WrapFiniteAction<cocos2d::MoveBy> MoveBy;
        inline MoveBy moveBy(const float duration, const cocos2d::Point &delta) { return MoveBy(duration, delta); }
        inline MoveBy moveBy(const float duration, const float dx, const float dy) { return moveBy(duration, cocos2d::Point(dx, dy)); }

        typedef Fluxion::WrapFiniteAction<cocos2d::FadeOut> FadeOut;
        inline FadeOut fadeOut(const float duration) { return FadeOut(duration); }

        typedef Fluxion::WrapFiniteAction<cocos2d::FadeIn> FadeIn;
        inline FadeIn fadeIn(const float duration) { return FadeIn(duration); }

        typedef Fluxion::WrapFiniteAction<cocos2d::FadeTo> FadeTo;
        inline FadeTo fadeTo(const float duration, const GLubyte opacity) { return FadeTo(duration, opacity); }

        typedef Fluxion::WrapFiniteAction<cocos2d::ScaleTo> ScaleTo;
        inline ScaleTo scaleTo(const float duration, const float s) { return ScaleTo(duration, s); }
        inline ScaleTo scaleTo(const float duration, const float sx, const float sy) { return ScaleTo(duration, sx, sy); }

        typedef Fluxion::WrapFiniteAction<cocos2d::ScaleBy> ScaleBy;
        inline ScaleBy scaleBy(const float duration, const float s) { return ScaleBy(duration, s); }
        inline ScaleBy scaleBy(const float duration, const float sx, const float sy) { return ScaleBy(duration, sx, sy); }

        typedef Fluxion::WrapFiniteAction<cocos2d::Blink> Blink;
        inline Blink blink(const float duration, const int blinks) { return Blink(duration, blinks); }

        typedef Fluxion::WrapFiniteAction<cocos2d::DelayTime> Wait;
        inline Wait wait(const float duration) { return Wait(duration); }
        inline Wait delayTime(const float duration) { return Wait(duration); }

        typedef Fluxion::WrapFiniteAction<cocos2d::RotateTo> RotateTo;
        inline RotateTo rotateTo(const float duration, const float angle) { return RotateTo(duration, angle); }
        inline RotateTo rotateTo(const float duration, const float angleX, const float angleY) { return RotateTo(duration, angleX, angleY); }

        typedef Fluxion::WrapFiniteAction<cocos2d::RotateBy> RotateBy;
        inline RotateBy rotateBy(const float duration, const float angle) { return RotateBy(duration, angle); }
        inline RotateBy rotateBy(const float duration, const float angleX, const float angleY) { return RotateBy(duration, angleX, angleY); }


        /* action */
        typedef Fluxion::WrapAction<cocos2d::Show> Show;
        inline Show show() { return Show(); }

        typedef Fluxion::WrapAction<cocos2d::Hide> Hide;
        inline Hide hide() { return Hide(); }

        typedef Fluxion::WrapAction<cocos2d::CallFunc> CallFunc;
        inline CallFunc call(const std::function<void()> &func) { return CallFunc(func); }

        template<typename... Args>
        inline Fluxion::ParallelAction p(const Args... args) { return Fluxion::ParallelAction::createFromActions(args...); }
        template<typename... Args>
        inline Fluxion::ParallelAction par(const Args... args) { return p(args...); }
        
        template<typename... Args>
        inline Fluxion::SequentialAction s(const Args... args) { return Fluxion::SequentialAction::createFromActions(args...); }
        template<typename... Args>
        inline Fluxion::SequentialAction seq(const Args... args) { return s(args...); }

        namespace Ease
        {
            /* ease */
            typedef Fluxion::WrapEase1<cocos2d::EaseIn, float> EaseIn;
            inline EaseIn easeIn(const float rate) { return EaseIn(rate); }

            typedef Fluxion::WrapEase1<cocos2d::EaseOut, float> EaseOut;
            inline EaseOut easeOut(const float rate) { return EaseOut(rate); }

            typedef Fluxion::WrapEase1<cocos2d::EaseInOut, float> EaseInOut;
            inline EaseInOut easeInOut(const float rate) { return EaseInOut(rate); }

            /* bounce */
            typedef Fluxion::WrapEase<cocos2d::EaseBounceIn> BounceIn;
            inline BounceIn bounceIn() { return BounceIn(); }

            typedef Fluxion::WrapEase<cocos2d::EaseBounceOut> BounceOut;
            inline BounceOut bounceOut() { return BounceOut(); }

            typedef Fluxion::WrapEase<cocos2d::EaseBounceInOut> BounceInOut;
            inline BounceInOut bounceInOut() { return BounceInOut(); }
            
            /* exp */
            typedef Fluxion::WrapEase<cocos2d::EaseExponentialIn> ExpIn;
            inline ExpIn expIn() { return ExpIn(); }

            typedef Fluxion::WrapEase<cocos2d::EaseExponentialOut> ExpOut;
            inline ExpOut expOut() { return ExpOut(); }

            typedef Fluxion::WrapEase<cocos2d::EaseExponentialInOut> ExpInOut;
            inline ExpInOut expInOut() { return ExpInOut(); }

            /* sign */
            typedef Fluxion::WrapEase<cocos2d::EaseSineIn> SineIn;
            inline SineIn sineIn() { return SineIn(); }

            typedef Fluxion::WrapEase<cocos2d::EaseSineOut> SineOut;
            inline SineOut sineOut() { return SineOut(); }

            typedef Fluxion::WrapEase<cocos2d::EaseSineInOut> SineInOut;
            inline SineInOut sineInOut() { return SineInOut(); }
        }
    }
}

#endif
