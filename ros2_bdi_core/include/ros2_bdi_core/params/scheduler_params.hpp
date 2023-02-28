#ifndef SCHEDULER_PARAMS_H_
#define SCHEDULER_PARAMS_H_

/* Parameters affecting internal logic (recompiling required) */
#define VAL_RESCHEDULE_POLICY_NO_IF_EXEC "NO_PREEMPT"
#define VAL_RESCHEDULE_POLICY_IF_EXEC "PREEMPT"
#define VAL_RESCHEDULE_POLICY_IF_EXEC_CLEAN "CLEAN_PREEMPT"

#define DESIRE_SET_TOPIC "desire_set"
#define ADD_DESIRE_TOPIC "add_desire"
#define BOOST_DESIRE_TOPIC "boost_desire"
#define DEL_DESIRE_TOPIC "del_desire"

#define INIT_DESIRE_SET_FILENAME "init_dset.yaml"

/*  Consider the plan almost completed if this threshold is surpassed by its progress status
    (useful to decide whether to abort the plan in case the target desire appears 
        to be fulfilled before the notification of the plan execution termination)
*/
#define COMPLETED_THRESHOLD 0.75 //TODO check in the future for a better value

//seconds to wait before giving up on performing a request (service does not appear to be up)
#define WAIT_SRV_UP 1   

//seconds to wait before giving up on waiting for the response
#define WAIT_RESPONSE_TIMEOUT 2

/* ROS2 Parameter names for PlanSys2Monitor node */
#define PARAM_MAX_TRIES_COMP_PLAN "comp_plan_tries"
#define PARAM_MAX_TRIES_EXEC_PLAN "exec_plan_tries"
#define PARAM_RESCHEDULE_POLICY "reschedule_policy"
#define PARAM_AUTOSUBMIT_PREC "autosub_prec"
#define PARAM_AUTOSUBMIT_CONTEXT "autosub_context"


#define CURR_INTENTIONS_TOPIC "current_intentions"

#define JAVAFF_SEARCH_TOPIC "javaff_search/plan"
#define JAVAFF_COMMITTED_STATUS_TOPIC "javaff_search/committed_status"
#define JAVAFF_EXEC_STATUS_TOPIC "javaff_search/exec_status"
#define JAVAFF_START_PLAN_SRV "javaff_server/start_plan"
#define JAVAFF_UNEXPECTED_STATE_SRV "javaff_server/unexpected_state"
#define JAVAFF_SEARCH_INTERVAL_PARAM "search_interval"
#define JAVAFF_MAX_PPLAN_SIZE_PARAM "max_pplan_size"
#define JAVAFF_SEARCH_MAX_EMPTY_SEARCH_INTERVALS_PARAM "max_null_search_intervals"

#define JAVAFF_SEARCH_INTERVAL_PARAM_DEFAULT 500
#define JAVAFF_SEARCH_MAX_EMPTY_SEARCH_INTERVALS_PARAM_DEFAULT 16
#define JAVAFF_MAX_PPLAN_SIZE_PARAM_DEFAULT 32000

#define PLAN_LIBRARY_NAME "plan_lib.db"

#endif