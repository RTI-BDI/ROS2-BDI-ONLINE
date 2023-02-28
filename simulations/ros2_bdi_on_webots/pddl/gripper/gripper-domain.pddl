;; domain file: gripper-domain.pddl

(define (domain gripper-domain)

    (:requirements :strips :typing :fluents :durative-actions)

    ;; Types ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    (:types
        box stackbase - box_or_stackbase
        deposit
        gripper
        carrier
    );; end Types ;;;;;;;;;;;;;;;;;;;;;;;;;

    (:predicates
        ( upon ?g - gripper ?s - stackbase)
        ( on ?b1 - box ?b2 - box_or_stackbase ?base - stackbase )
        ( in ?sb1 - box_or_stackbase ?sb2 - box_or_stackbase )
        ( clear ?b - box_or_stackbase )
        ( holding ?g - gripper ?b - box )
        ( stored ?b - box ?dep - deposit )

        ( carrier_in_base ?c - carrier ?sb - stackbase)
        ( carrier_in_deposit ?c - carrier ?dp - deposit)
        ( carrier_can_come ?c - carrier ?sb - stackbase)
        ( carrier_can_go ?c - carrier ?dp - deposit)
        ( carrier_moving ?c - carrier ?b - box)
    )

    (:functions
        (holding_boxes ?g - gripper)
        (stacked ?base - stackbase)
        (moving_boxes ?c - carrier)
    )

    (:durative-action gripper_pickup
        :parameters (?g - gripper ?b1 - box ?bs2 - box_or_stackbase ?base - stackbase)
        :duration (= ?duration 3)
        :condition (and
            (at start (on ?b1 ?bs2 ?base))
            (at start (in ?b1 ?base))
            (at start (in ?bs2 ?base))
            (at start (clear ?b1))
            (at start (< (holding_boxes ?g) 1))
            (over all (upon ?g ?base))
        )
        :effect (and
            (at start (increase (holding_boxes ?g) 1))
            (at end (not(in ?b1 ?base)))
            (at end (holding ?g ?b1))
            (at end (not(clear ?b1)))
            (at end (not(on ?b1 ?bs2 ?base)))
            (at end (clear ?bs2))
            (at end (decrease (stacked ?base) 1))
        )
    )

    (:durative-action gripper_putdown
        :parameters (?g - gripper ?b1 - box ?bs2 - box_or_stackbase ?base - stackbase)
        :duration (= ?duration 3)
        :condition (and
            (at start (clear ?bs2))
            (at start (holding ?g ?b1))
            (over all (upon ?g ?base))
            (at start (< (stacked ?base) 3))
            (at start (in ?bs2 ?base))
        )
        :effect (and
            (at start (not(clear ?bs2)))
            (at end (decrease (holding_boxes ?g) 1))
            (at end (increase (stacked ?base) 1))
            (at end (not (holding ?g ?b1)))
            (at end (clear ?b1))
            (at end (on ?b1 ?bs2 ?base))
            (at end (in ?b1 ?base))
        )
    )

    (:durative-action req_carrier_to_come
        :parameters (?c - carrier ?dep - deposit ?base - stackbase)
        :duration (= ?duration 6)
        :condition (and
            (at start (carrier_in_deposit ?c ?dep))
            (over all (clear ?base))
            (over all (carrier_can_come ?c ?base))
        )
        :effect (and
            (at start (not (carrier_in_deposit ?c ?dep)))
            (at end (not(clear ?base)))
            (at end (carrier_in_base ?c ?base))
        )
    )

    (:durative-action gripper_put_on_carrier
        :parameters (?g - gripper ?b - box ?c - carrier ?base - stackbase)
        :duration (= ?duration 4)
        :condition (and
            (at start (< (moving_boxes ?c) 2))
            (at start (holding ?g ?b))
            (at start (upon ?g ?base))
            (at start (carrier_in_base ?c ?base))
            (over all (upon ?g ?base))
            (over all (carrier_in_base ?c ?base))
        )
        :effect (and
            (at end (decrease (holding_boxes ?g) 1))
            (at end (increase (moving_boxes ?c) 1))
            (at end (not (holding ?g ?b)))
            (at end (carrier_moving ?c ?b))
        )
    )

    (:durative-action gripper_move
        :parameters (?g - gripper ?sb1 ?sb2 - stackbase)
        :duration (= ?duration 5)
        :condition (and
            (at start (upon ?g ?sb1))
        )
        :effect (and
            (at start (not(upon ?g ?sb1)))
            (at end (upon ?g ?sb2))
        )
    )
)
