#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

#include <string>
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <mutex> 

#include "plansys2_domain_expert/DomainExpertClient.hpp" 

#include "ros2_bdi_interfaces/msg/lifecycle_status.hpp"
#include "ros2_bdi_interfaces/msg/belief.hpp"
#include "ros2_bdi_interfaces/msg/belief_set.hpp"
#include "ros2_bdi_interfaces/msg/desire.hpp"
#include "ros2_bdi_interfaces/msg/desire_set.hpp"

#include "ros2_bdi_utils/ManagedBelief.hpp"
#include "ros2_bdi_utils/ManagedDesire.hpp"
#include "ros2_bdi_utils/ManagedCondition.hpp"
#include "ros2_bdi_utils/ManagedReactiveRule.hpp"

#include "ros2_bdi_utils/BDIFilter.hpp"

#include "ros2_bdi_core/params/core_common_params.hpp"
#include "ros2_bdi_core/params/event_listener_params.hpp"
#include "ros2_bdi_core/support/planning_mode.hpp"
#include "ros2_bdi_core/support/plansys_monitor_client.hpp"

#include "rclcpp/rclcpp.hpp"

typedef enum {STARTING, CHECKING} StateType;   
/*
    Checks at every belief set update for conditions to be satisfied,
    if any of them is sat, then apply the corresponding rules

    If no rules are defined at start, the node automatically shuts down 
*/
class EventListener : public rclcpp::Node
{
    public:

        /* Constructor method */
        EventListener();

        /*
            Init to call at the start, after construction method, to get the node actually started
            initializing publishers to add/del belief/desire topics of the agent
            and subscriber to the belief_set topic.
            Policies to check are retrieved and load from a static file and cannot be modified at run time (yet)

            Returns false if no reactive rule were to be found
        */
        bool init();

        /*
            Wait for PlanSys2 to boot at best for max_wait
        */
        bool wait_psys2_boot(const std::chrono::seconds max_wait = std::chrono::seconds(16))
        {
            psys_monitor_client_ = std::make_shared<PlanSysMonitorClient>(EVENT_LISTENER_NODE_NAME + std::string("_psys2caller_"), sel_planning_mode_);
            return psys_monitor_client_->areAllPsysNodeActive(max_wait);
        }

    private:

        /*Build updated ros2_bdi_interfaces::msg::LifecycleStatus msg*/
        ros2_bdi_interfaces::msg::LifecycleStatus getLifecycleStatus();
        
        /* Callback of belief set update -> if something changes and you've correctly booted, check if any rule applies*/
        void updBeliefSetCallback(const ros2_bdi_interfaces::msg::BeliefSet::SharedPtr msg);

        /* Callback of desire set update */
        void updDesireSetCallback(const ros2_bdi_interfaces::msg::DesireSet::SharedPtr msg)
        {
            desire_set_ = BDIFilter::extractMGDesires(msg->value);
        }

        /*
            Received notification about ROS2-BDI Lifecycle status
        */
        void callbackLifecycleStatus(const ros2_bdi_interfaces::msg::LifecycleStatus::SharedPtr msg)
        {
            if(msg->node_name == BELIEF_MANAGER_NODE_NAME && msg->status == ros2_bdi_interfaces::msg::LifecycleStatus{}.RUNNING)
                state_ = CHECKING; // start checking if any rule applies when belief manager has successfully terminated boot 
    
            if(lifecycle_status_.find(msg->node_name) != lifecycle_status_.end())//key in map, record upd value
                lifecycle_status_[msg->node_name] = msg->status;
        }

        // recover from expected file the rules to be applied
        std::set<BDIManaged::ManagedReactiveRule> init_reactive_rules();

        /*Iterate over the rules and check if any of them applies, if yes enforces it*/
        void check_if_any_rule_apply();
        
        /*Apply reactive rule, by publishing to the right topic belief/desire set updates as defined in reactive_rule*/
        void apply_rule(const BDIManaged::ManagedReactiveRule& reactive_rule);

        /*Apply satisfying rules starting from extracted possible assignments for the placeholders*/
        void apply_satisfying_rules(const BDIManaged::ManagedReactiveRule& reactive_rule, 
            const std::map<std::string, std::vector<BDIManaged::ManagedBelief>>& assignments, 
            const std::set<BDIManaged::ManagedBelief>& belief_set);

        // internal state of the node
        StateType state_; 
               
        // agent id that defines the namespace in which the node operates
        std::string agent_id_;
        // step counter
        uint64_t step_counter_;

        // Selected planning mode
        PlanningMode sel_planning_mode_;

        //policy rules set
        std::set<BDIManaged::ManagedReactiveRule> reactive_rules_;


        // domain expert instance to call the plansys2 domain expert api
        std::shared_ptr<plansys2::DomainExpertClient> domain_expert_;

        std::set<BDIManaged::ManagedBelief> belief_set_;
        std::set<BDIManaged::ManagedDesire> desire_set_;

        // belief set publishers
        rclcpp::Publisher<ros2_bdi_interfaces::msg::Belief>::SharedPtr add_belief_publisher_;//add belief topic pub
        rclcpp::Publisher<ros2_bdi_interfaces::msg::Belief>::SharedPtr del_belief_publisher_;//del belief topic pub

        // desire publishers
        rclcpp::Publisher<ros2_bdi_interfaces::msg::Desire>::SharedPtr add_desire_publisher_;//add desire topic pub
        rclcpp::Publisher<ros2_bdi_interfaces::msg::Desire>::SharedPtr boost_desire_publisher_;//boost desire topic pub
        rclcpp::Publisher<ros2_bdi_interfaces::msg::Desire>::SharedPtr del_desire_publisher_;//del desire topic pub

        rclcpp::Subscription<ros2_bdi_interfaces::msg::BeliefSet>::SharedPtr belief_set_subscription_;//belief set subscription
        rclcpp::Subscription<ros2_bdi_interfaces::msg::DesireSet>::SharedPtr desire_set_subscription_;//desire set subscription


        // current known status of the system nodes
        std::map<std::string, uint8_t> lifecycle_status_;
        // Publish updated lifecycle status
        rclcpp::Publisher<ros2_bdi_interfaces::msg::LifecycleStatus>::SharedPtr lifecycle_status_publisher_;
        // Sub to updated lifecycle status
        rclcpp::Subscription<ros2_bdi_interfaces::msg::LifecycleStatus>::SharedPtr lifecycle_status_subscriber_;

        // PlanSys2 Monitor Client supporting nodes & clients for calling the {psys2_node}/get_state services
        std::shared_ptr<PlanSysMonitorClient> psys_monitor_client_;

}; //BeliefManager class prototype

#endif //EVENT_LISTENER_H_
