// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _ONEDPL_ROUND_ROBIN_POLICY_IMPL_H
#define _ONEDPL_ROUND_ROBIN_POLICY_IMPL_H

#include <atomic>
#include <type_traits>
#include <vector>
#include <memory>
#include <exception>
#include <limits>
#include "oneapi/dpl/internal/dynamic_selection_impl/scoring_policy_defs.h"
#if _DS_BACKEND_SYCL != 0
    #include "oneapi/dpl/internal/dynamic_selection_impl/sycl_backend.h"
#endif

namespace oneapi {
namespace dpl{
namespace experimental{
#if _DS_BACKEND_SYCL != 0
  template <typename Backend=sycl_backend>
#else
  template <typename Backend>
#endif
  struct round_robin_policy {
  private:
    using backend_t = Backend;
    using resource_container_t = typename backend_t::resource_container_t;
    using resource_container_size_t = typename resource_container_t::size_type;
    using wrapped_resource_t = typename std::decay_t<Backend>::execution_resource_t;

    using execution_resource_t = typename backend_t::execution_resource_t;

  public:
    //Policy Traits
    public:
    using selection_type = oneapi::dpl::experimental::basic_selection_handle_t<round_robin_policy<Backend>, execution_resource_t>;
    using resource_type = decltype(unwrap(std::declval<wrapped_resource_t>()));
    using wait_type = typename backend_t::wait_type;

  private:
    std::shared_ptr<backend_t> backend_;

    struct state_t{
        resource_container_t resources_;
        resource_container_size_t num_contexts_;
        std::atomic<resource_container_size_t> next_context_;
    };

    std::shared_ptr<state_t> state_;

    public:
    auto get_resources() const {
        if(backend_){
            return backend_->get_resources();
        }else{
            throw std::logic_error("Called get_resources before initialization\n");
        }
    }

    void initialize() {
      if(!state_){
          backend_ = std::make_shared<backend_t>();
          state_= std::make_shared<state_t>();
          state_->resources_ = get_resources();
          state_->num_contexts_ = state_->resources_.size();
          state_->next_context_ = 0;
      }
    }

    void initialize(const std::vector<resource_type>& u) {
      if(!state_){
          backend_ = std::make_shared<backend_t>(u);
          state_= std::make_shared<state_t>();
          for(auto x : u){
              state_->resources_.push_back(x);
          }
          state_->num_contexts_ = state_->resources_.size();
          state_->next_context_=0;
      }
    }

    round_robin_policy() {
        initialize();
    }

    round_robin_policy(deferred_initialization) {}

    round_robin_policy(const std::vector<resource_type>& u) {
        initialize(u);
    }

    template<typename ...Args>
    selection_type select(Args&&...) {
      if(state_){
          size_t i=0;
          while(true){
              resource_container_size_t current_context_ = state_->next_context_.load();
              resource_container_size_t new_context_;
              if(current_context_ == std::numeric_limits<resource_container_size_t>::max()){
                  new_context_ = (current_context_%state_->num_contexts_)+1;
              }
              else{
                  new_context_ = (current_context_+1)%state_->num_contexts_;
              }

              if(state_->next_context_.compare_exchange_weak(current_context_, new_context_)){
                  i = current_context_;
                  break;
              }
          }
          auto &e = state_->resources_[i];
          return selection_type{*this, e};
      }else{
        throw std::logic_error("Called select before initialization\n");
      }
    }

    template<typename Function, typename ...Args>
    auto submit(selection_type e, Function&& f, Args&&... args) {
      if(backend_){
        return backend_->submit(e, std::forward<Function>(f), std::forward<Args>(args)...);
      }else{
        throw std::logic_error("Called submit before initialization\n");
      }
    }

    auto get_submission_group() {
      if(backend_){
        return backend_->get_submission_group();
      }else{
        throw std::logic_error("Called get_submission_group before initialization\n");
      }
    }

  };
} // namespace experimental

} // namespace dpl

} // namespace oneapi


#endif //_ONEDPL_ROUND_ROBIN_POLICY_IMPL_H
