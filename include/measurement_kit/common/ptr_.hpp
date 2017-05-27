// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_PTR__HPP
#define MEASUREMENT_KIT_COMMON_PTR__HPP

#include <memory>
#include <stdexcept>

#define MK_DEFINE_PTR_(PtrName, SmartPtr, SmartPtrMaker)                       \
    template <typename T> class PtrName : public std::SmartPtr<T> {            \
        using std::SmartPtr<T>::SmartPtr;                                      \
                                                                               \
      public:                                                                  \
        typename std::add_pointer<T>::type get() const {                       \
            return operator->();                                               \
        }                                                                      \
                                                                               \
        typename std::add_pointer<T>::type operator->() const {                \
            if (std::SmartPtr<T>::get() == nullptr) {                          \
                throw std::runtime_error("null pointer");                      \
            }                                                                  \
            return std::SmartPtr<T>::operator->();                             \
        }                                                                      \
                                                                               \
        typename std::add_lvalue_reference<T>::type operator*() const {        \
            if (std::SmartPtr<T>::get() == nullptr) {                          \
                throw std::runtime_error("null pointer");                      \
            }                                                                  \
            return std::SmartPtr<T>::operator*();                              \
        }                                                                      \
                                                                               \
        template <typename... A> static PtrName<T> make(A &&... a) {           \
            return std::SmartPtrMaker<T>(std::forward<A>(a)...);               \
        }                                                                      \
                                                                               \
      protected:                                                               \
      private:                                                                 \
        /* NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE */ \
        /* DOING THAT CREATES THE RISK OF OBJECT SLICING. */                   \
    }

#endif
