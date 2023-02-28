#ifndef MANAGED_PLAN_H_
#define MANAGED_PLAN_H_

#include <string>
#include <vector>
#include <iostream>

#include "plansys2_planner/PlannerClient.hpp"
#include "plansys2_msgs/msg/plan_item.hpp"
#include "plansys2_msgs/msg/plan.hpp"
#include "ros2_bdi_interfaces/msg/bdi_plan.hpp"
#include "ros2_bdi_interfaces/msg/bdi_action_execution_info.hpp"
#include "ros2_bdi_interfaces/msg/bdi_plan_execution_info.hpp"
#include "ros2_bdi_utils/ManagedBelief.hpp"
#include "ros2_bdi_utils/ManagedDesire.hpp"


/* Namespace for wrapper classes wrt. BDI msgs defined in ros2_bdi_interfaces::msg */
namespace BDIManaged
{

    /* Wrapper class to easily manage and infer info from a ros2_bdi_interfaces::msg::Plan instance*/
    class ManagedPlan
    {

        public:
            /* Constructor methods */
            ManagedPlan();
            ManagedPlan(const int16_t& plan_index, const ManagedDesire& md, const std::vector<plansys2_msgs::msg::PlanItem>& planitems);
            ManagedPlan(const int16_t& plan_index, const ManagedDesire& md, const std::vector<plansys2_msgs::msg::PlanItem>& planitems, 
                const ManagedConditionsDNF& precondition, const ManagedConditionsDNF& context);
            ManagedPlan(const int16_t& plan_index, const ManagedDesire& finalDesire, const ManagedDesire& intermediateDesire,
                const std::vector<plansys2_msgs::msg::PlanItem>& planitems, const ManagedConditionsDNF& precondition, 
                const ManagedConditionsDNF& context);
            
            /* getter methods for ManagedPlan instance prop  */
            ManagedDesire getFinalTarget() const {return *final_target_;}
            ManagedDesire getPlanTarget() const {return *plan_target_;}

            /* getter/setter methods for ManagedPlan instance prop  */
            int getPlanLibID() const {return planlib_id_;}
            void setPlanLibID(const int& id) {planlib_id_ = id;}

            int16_t getPlanQueueIndex() const {return planqueue_index_;}

            void setUpdatedInfo(const ros2_bdi_interfaces::msg::BDIPlanExecutionInfo& planExecInfo)
            {
                this->exec_status_ = planExecInfo.status;
                this->last_current_time_ = planExecInfo.current_time;
                std::vector<ros2_bdi_interfaces::msg::BDIActionExecutionInfo> actions_exec_info = 
                    std::vector<ros2_bdi_interfaces::msg::BDIActionExecutionInfo> (planExecInfo.actions_exec_info.begin(), planExecInfo.actions_exec_info.end());
                // do not lose current committed status
                if(actions_exec_info.size() == actions_exec_info_.size())
                    for(int i=0; i<actions_exec_info_.size(); i++)
                        actions_exec_info[i].committed = actions_exec_info_[i].committed;
                this->actions_exec_info_ = actions_exec_info;
            }

            void setCommittedStatus(const bool& defaultValue)
            {
                for(int i=0; i<actions_exec_info_.size(); i++)
                    actions_exec_info_[i].committed = defaultValue;
            }

            void setActionCommittedStatus(const std::string& action_name, const float& planned_time, const bool& committed)
            {
                std::string action_name_timex1000 = buildFullActionNameTimex1000(action_name, planned_time);
                for(int i=0; i<actions_exec_info_.size(); i++)
                {
                    if(action_name_timex1000 == buildFullActionNameTimex1000(
                        buildFullActionName(actions_exec_info_[i].name, actions_exec_info_[i].args),
                        actions_exec_info_[i].planned_start)
                    )
                        actions_exec_info_[i].committed = committed;
                }
            }

            plansys2_msgs::msg::Plan getActionCommittedStatus()
            {
                plansys2_msgs::msg::Plan committedPlan;
                committedPlan.plan_index = getPlanQueueIndex();
                for(int i=0; i<actions_exec_info_.size(); i++)
                {
                    plansys2_msgs::msg::PlanItem action;
                    std::string joinedArgs = "";
                    for(std::string arg : actions_exec_info_[i].args)
                        joinedArgs += " " + arg;
                    action.action = "(" + actions_exec_info_[i].name + joinedArgs + ")";
                    action.time = actions_exec_info_[i].planned_start;
                    action.duration = actions_exec_info_[i].duration;
                    action.committed = actions_exec_info_[i].committed;
                    committedPlan.items.push_back(action);
                }
                return committedPlan;
            }

            std::vector<ros2_bdi_interfaces::msg::BDIActionExecutionInfo> getActionsExecInfo() const {return actions_exec_info_;};
            float getPlannedDeadline() const {return planned_deadline_;};
            
            /*Already started/executing actions start/end time taken in consideration for estimating the new deadline at run time*/
            //TODO fix the logics!!!
            float getUpdatedEstimatedDeadline();

            ManagedConditionsDNF getPrecondition() const {return precondition_;};
            ManagedConditionsDNF getContext() const {return context_;};
            
            /* convert instance to plansys2::msg::Plan format */
            plansys2_msgs::msg::Plan toPsys2Plan() const;

            /* convert instance to ros2_bdi_interfaces::msg::BDIPlan format*/
            ros2_bdi_interfaces::msg::BDIPlan toPlan() const;

            /*
                Example 
                From action name "a1" and args ["p1", "p2"]
                return "(a1 p1 p2)"
            */
            static std::string computeActionFullName(ros2_bdi_interfaces::msg::BDIActionExecutionInfo action_exec);

            /*
                Try to parse an array of psys2 plan items from a string, format is the following

                [start_time](action)[duration]\n[start_time](action)[duration]
            */
            static std::optional<std::vector<plansys2_msgs::msg::PlanItem>> parsePsys2PlanMsg(std::string plan_msg);

            /*
                Convert managed plan to Plansys2 msg string, format is the following

                [start_time](action)[duration]\n[start_time](action)[duration]
            */
            std::string toPsys2PlanString() const;

        private:

            /*
                compute actoin exec info from plan item, execution status UNKNOWN
            */
            static std::vector<ros2_bdi_interfaces::msg::BDIActionExecutionInfo> 
                computeActionsExecInfo(std::vector<plansys2_msgs::msg::PlanItem> plan_items);

            
            static std::string buildFullActionName(const std::string& action_name, const std::vector<std::string>& args)
            {
                std::string result = std::string{action_name};
                for(uint16_t i = 0; i < args.size(); i++)
                    result += " " + args[i];
                return "(" + result + ")";
            }
            
            static std::string buildFullActionNameTimex1000(const std::string& action_name, const float& planned_time)
            {
                std::string result = std::string{action_name};
                if(result.find("(") != std::string::npos)
                    result = result.substr(result.find("(")+1);
                if(result.find(")") != std::string::npos)
                    result = result.substr(0,result.find(")"));
               int ptimex1000 = static_cast<int>(planned_time * 1000);
               return "(" + result + "):" + std::to_string(ptimex1000);
            }

            /* Compute deadline estimate based on current actions estimated duration within the one listed in the plan */
            float computePlannedDeadline();

            float computeUpdatedEndTime(const ros2_bdi_interfaces::msg::BDIActionExecutionInfo& bdi_ai);

            /* Desire to be fulfilled after successful plan execution*/
            std::shared_ptr<ManagedDesire> plan_target_;

            /* Main desire that is currently under pursuit and the plan execution should increment the possibilities to fulfill it*/
            std::shared_ptr<ManagedDesire> final_target_;

            /* Plansys2 action (name, duration, start time) vector enwrapping the tree of actions to be performed to fulfilled the desire
                that needs to be passed to PlanSys2 Executor */
            std::vector<ros2_bdi_interfaces::msg::BDIActionExecutionInfo> actions_exec_info_;

            /* Condition clauses in a DNF expression that must be verified before plan exec starts */
            ManagedConditionsDNF precondition_;

            /* Condition clauses in a DNF expression that must be verified over all plan exec */
            ManagedConditionsDNF context_;

            /* Planned deadline */
            float planned_deadline_;

            /* Last reported current time */
            float last_current_time_;

            /* Plan level exec status*/
            std::string exec_status_;

            /*If stored in plan lib, id with which it was stored*/
            int planlib_id_ = -1;

            /*Index in queue of partial plans (just for online planning mode) wrt. current global target and plans execution*/
            int16_t planqueue_index_ = -1;

    };  // class ManagedPlan

    bool operator==(ManagedPlan const &mp1, ManagedPlan const &mp2);
    bool operator!=(ManagedPlan const &mp1, ManagedPlan const &mp2);
    std::ostream& operator<<(std::ostream& os, const ManagedPlan& mp);

}

#endif  // MANAGED_PLAN_H_