#ifndef PLAN_DIRECTOR_H_
#define PLAN_DIRECTOR_H_

#include <string>
#include <vector>
#include <set>
#include <map>
#include <optional>
#include <memory>
#include <chrono>


#include "plansys2_domain_expert/DomainExpertClient.hpp"
#include "plansys2_problem_expert/ProblemExpertClient.hpp"
#include "plansys2_executor/ExecutorClient.hpp"

#include "ros2_bdi_interfaces/msg/lifecycle_status.hpp"
#include "ros2_bdi_interfaces/msg/belief.hpp"
#include "ros2_bdi_interfaces/msg/belief_set.hpp"
#include "ros2_bdi_interfaces/msg/desire.hpp"
#include "ros2_bdi_interfaces/msg/planning_system_state.hpp"
#include "ros2_bdi_interfaces/msg/bdi_action_execution_info.hpp"
#include "ros2_bdi_interfaces/msg/bdi_plan_execution_info.hpp"

#include "ros2_bdi_interfaces/srv/bdi_plan_execution.hpp"
#include "ros2_bdi_utils/ManagedBelief.hpp"
#include "ros2_bdi_utils/ManagedPlan.hpp"

#include "ros2_bdi_core/params/core_common_params.hpp"
#include "ros2_bdi_core/params/plan_director_params.hpp"
#include "ros2_bdi_core/support/plansys_monitor_client.hpp"
#include "ros2_bdi_core/support/planning_mode.hpp"

#include "rclcpp/rclcpp.hpp"

typedef enum {STARTING, READY, EXECUTING, PAUSE} StateType;      

class PlanDirector : public rclcpp::Node
{
public:
  
    /* Constructor method */
    PlanDirector();

    /*
        Init to call at the start, after construction method, to get the node actually started
        initialing psys2 executor client instance, 
        retrieving agent_id_ (thus namespace)
        defining work timer
    */
    void init();
    
    /*
        Main loop of work called regularly through a wall timer
    */
    void step();

    /*
        Wait for PlanSys2 to boot at best for max_wait
    */
    bool wait_psys2_boot(const std::chrono::seconds max_wait = std::chrono::seconds(16))
    {
        psys_monitor_client_ = std::make_shared<PlanSysMonitorClient>(PLAN_DIRECTOR_NODE_NAME + std::string("_psys2caller_"), sel_planning_mode_);
        return psys_monitor_client_->areAllPsysNodeActive(max_wait);
    }

private:
    /*  
        Change internal state of the node
    */
    void setState(StateType state){ state_ = state;  }

    // clear info about current plan execution
    void setNoPlanMsg(){ current_plan_ = BDIManaged::ManagedPlan{}; }

    /*
        Check planExecution feedback from plansys2 executor + check for context conditions
    */
    void executingPlan();

    /*
        When in READY state, msg to publish in plan_execution_info to notify it 
        (i.e. notify you're not executing any plan)
    */
    void publishNoPlanExec();
    
    /*
       Received notification about PlanSys2 nodes state by plansys2 monitor node
    */
    void callbackPsys2State(const ros2_bdi_interfaces::msg::PlanningSystemState::SharedPtr msg);
    
    /*Build updated ros2_bdi_interfaces::msg::LifecycleStatus msg*/
    ros2_bdi_interfaces::msg::LifecycleStatus getLifecycleStatus();

    /*
        Received notification about ROS2-BDI Lifecycle status
    */
    void callbackLifecycleStatus(const ros2_bdi_interfaces::msg::LifecycleStatus::SharedPtr msg)
    {
        if(lifecycle_status_.find(msg->node_name) != lifecycle_status_.end())//key in map, record upd value
            lifecycle_status_[msg->node_name] = msg->status;
    }

    /*
        Currently executing no plan
    */
    bool executingNoPlan();

    /*
        Cancel current plan execution (if any) and information preserved in it
    */
    bool cancelCurrentPlanExecution();


    /*
        Start new plan execution -> true if correctly started
    */
    bool startPlanExecution(const BDIManaged::ManagedPlan& mp);



    // redefine workint timer interval
    void resetWorkTimer(const int& ms);

    /*
        Return true if plan exec request is well formed 
            - request = ABORT | EXECUTE
            - at least a planitem elem in body
            - for each plan item action
                - check its definition exists with domain expert
                - check its params are valid instances (problem expert) matching the correct types (domain expert)
    */
    bool validPlanRequest(const ros2_bdi_interfaces::srv::BDIPlanExecution::Request::SharedPtr request);

    /*  
        Callback to handle the service request to trigger a new plan execution or abort the current one
    */
    void handlePlanRequest(const ros2_bdi_interfaces::srv::BDIPlanExecution::Request::SharedPtr request,
        const ros2_bdi_interfaces::srv::BDIPlanExecution::Response::SharedPtr response);

    /*
        Plan currently in execution, monitor and publish the feedback of its development
    */
    void checkContextConditions();

    /* 
        publish beliefs to be added and beliefs to be deleted as a consequence of plan abortion (rollback)
    */
    void publishRollbackBeliefs(const std::vector<ros2_bdi_interfaces::msg::Belief> rollback_belief_add, 
        const std::vector<ros2_bdi_interfaces::msg::Belief> rollback_belief_del);


    /*
        Plan currently in execution, monitor and publish the feedback of its development
    */
    void checkPlanExecution(const bool& force_update = false);

    /* 
        Use PlanSys2 feedback received from the executor to build the BDIPlanExecutionInfo to be published to the respecive topic
        Call NECESSARY to update the properties regarding the status of the current monitored/managed plan exec.
    */
    ros2_bdi_interfaces::msg::BDIPlanExecutionInfo getPlanExecutionInfo(const plansys2::ExecutorClient::ExecutePlan::Feedback& feedback);


    /*
    Retrieve from PlanSys2 Executor status info about current plan execution: RUNNING, SUCCESSFUL, ABORT
    */
    int16_t getPlanExecutionStatus();


    /*
        The belief set has been updated
    */
    void updatedBeliefSet(const ros2_bdi_interfaces::msg::BeliefSet::SharedPtr msg);

    // internal state of the node
    StateType state_;

    // Selected planning mode
    PlanningMode sel_planning_mode_;
    
    // agent id that defines the namespace in which the node operates
    std::string agent_id_;
    // step counter
    uint64_t step_counter_;

    // timer to trigger callback to perform main loop of work regularly
    rclcpp::TimerBase::SharedPtr do_work_timer_;

    // counter of communication errors with plansys2
    int psys2_comm_errors_;
    // domain expert client contacting psys2 for checking validity of a plan
    std::shared_ptr<plansys2::DomainExpertClient> domain_expert_client_;
    // problem expert client contacting psys2 for checking validity of a plan
    std::shared_ptr<plansys2::ProblemExpertClient> problem_expert_client_;
    // executor client contacting psys2 for the execution of a plan, then receiving feedback for it 
    std::shared_ptr<plansys2::ExecutorClient> executor_client_;

    // flag to denote if plansys2 domain expert appears to be active
    bool psys2_domain_expert_active_;
    // flag to denote if plansys2 problem expert appears to be active
    bool psys2_problem_expert_active_;
    // flag to denote if plansys2 executor appears to be active
    bool psys2_executor_active_;
    // plansys2 node status monitor subscription
    rclcpp::Subscription<ros2_bdi_interfaces::msg::PlanningSystemState>::SharedPtr plansys2_status_subscriber_;

    // current_plan_ in execution (could be none if the agent isn't doing anything)
    BDIManaged::ManagedPlan current_plan_;
    // # checks performed during the current plan exec
    int counter_check_;
    // time at which plan started (NOT DOING this anymore -> using first start_ts from first action executed in plan)
    //high_resolution_clock::time_point current_plan_start_;
    // msg to notify the idle-ready state, i.e. no current plan execution, but ready to do it
    ros2_bdi_interfaces::msg::BDIPlanExecutionInfo no_plan_msg_;

    // current belief set (in order to check precondition && context condition)
    std::set<BDIManaged::ManagedBelief> belief_set_;
    // belief set subscriber
    rclcpp::Subscription<ros2_bdi_interfaces::msg::BeliefSet>::SharedPtr belief_set_subscriber_;//belief set sub.
    // belief add publisher
    rclcpp::Publisher<ros2_bdi_interfaces::msg::Belief>::SharedPtr belief_add_publisher_;
    // belief del publisher
    rclcpp::Publisher<ros2_bdi_interfaces::msg::Belief>::SharedPtr belief_del_publisher_;

    // record first timestamp in sec of the current plan execution (to subtract from it)
    int first_ts_plan_sec_;
    unsigned int first_ts_plan_nanosec_;
    // last recorded timestamp during plan execution
    float last_ts_plan_exec_;

    // notification about the current plan execution -> plan execution info publisher
    rclcpp::Publisher<ros2_bdi_interfaces::msg::BDIPlanExecutionInfo>::SharedPtr plan_exec_publisher_;

    // trigger plan execution/abortion service
    rclcpp::Service<ros2_bdi_interfaces::srv::BDIPlanExecution>::SharedPtr server_plan_exec_;

    // current known status of the system nodes
    std::map<std::string, uint8_t> lifecycle_status_;
    // Publish updated lifecycle status
    rclcpp::Publisher<ros2_bdi_interfaces::msg::LifecycleStatus>::SharedPtr lifecycle_status_publisher_;
    // Sub to updated lifecycle status
    rclcpp::Subscription<ros2_bdi_interfaces::msg::LifecycleStatus>::SharedPtr lifecycle_status_subscriber_;

    // PlanSys2 Monitor Client supporting nodes & clients for calling the {psys2_node}/get_state services
    std::shared_ptr<PlanSysMonitorClient> psys_monitor_client_;

}; // PlanDirector class prototype

#endif // PLAN_DIRECTOR