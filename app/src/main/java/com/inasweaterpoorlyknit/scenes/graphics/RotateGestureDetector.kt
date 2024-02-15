package com.inasweaterpoorlyknit.scenes.graphics

import android.view.MotionEvent
import com.inasweaterpoorlyknit.Vec2

class RotateGestureDetector {
    data class Pointer(
        var id: Int = MotionEvent.INVALID_POINTER_ID,
        var pos: Vec2 = Vec2(-1f, -1f),
        var startingPos: Vec2 = Vec2(-1f, -1f)
    )

    private var pointer1 = Pointer()
    private var pointer2 = Pointer()
    private var previousLifetimeRotation: Float = 0f // radians
    private var currentGestureRotation: Float = 0f
    val lifetimeRotation
        get() = previousLifetimeRotation + currentGestureRotation
    val isActive: Boolean
        get() = pointer1.id != MotionEvent.INVALID_POINTER_ID && pointer2.id != MotionEvent.INVALID_POINTER_ID
    private val centerFocus: Vec2
        get() = Vec2((pointer1.pos.x + pointer2.pos.x) * .5f, (pointer1.pos.y + pointer2.pos.y) * .5f)

    private fun updateTotalRotation() {
        currentGestureRotation = angleBetweenLines(pointer1.startingPos, pointer2.startingPos, pointer1.pos, pointer2.pos)
    }

    fun onTouchEvent(event: MotionEvent) {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                val eventPointerId = event.getPointerId(event.actionIndex)
                pointer1.id = eventPointerId
                pointer1.startingPos = Vec2(event.x, event.y)
                pointer1.pos = Vec2(pointer1.startingPos.x, pointer1.startingPos.y)
            }
            MotionEvent.ACTION_MOVE -> {
                if (isActive) {
                    val pointerIndex1 = event.findPointerIndex(pointer1.id)
                    val pointerIndex2 = event.findPointerIndex(pointer2.id)
                    if(pointerIndex1 != MotionEvent.INVALID_POINTER_ID && pointerIndex2 != MotionEvent.INVALID_POINTER_ID) { // if pointers are still valid
                        pointer1.pos = Vec2(event.getX(), event.getY(event.findPointerIndex(pointer1.id)))
                        pointer2.pos = Vec2(event.getX(event.findPointerIndex(pointer2.id)), event.getY(event.findPointerIndex(pointer2.id)))
                        updateTotalRotation()
                    }
                }
            }
            MotionEvent.ACTION_UP -> {
                val eventPointerId = event.getPointerId(event.actionIndex)
                if(pointer1.id == eventPointerId) {
                    pointer1.id = MotionEvent.INVALID_POINTER_ID
                } else if(pointer2.id == eventPointerId) {
                    pointer2.id = MotionEvent.INVALID_POINTER_ID
                }
            }
            MotionEvent.ACTION_POINTER_DOWN -> {
                val eventPointerId = event.getPointerId(event.actionIndex)
                if(pointer1.id == MotionEvent.INVALID_POINTER_ID || pointer2.id == MotionEvent.INVALID_POINTER_ID) {
                    if (pointer1.id == MotionEvent.INVALID_POINTER_ID) {
                        val pointer2Temp = pointer2.copy()
                        pointer2 = pointer1.copy()
                        pointer1 = pointer2Temp
                    } else {
                        pointer2.id = eventPointerId
                    }
                    val pointerIndex1 = event.findPointerIndex(pointer1.id)
                    val pointerIndex2 = event.findPointerIndex(pointer2.id)
                    if (pointerIndex1 != MotionEvent.INVALID_POINTER_ID && pointerIndex2 != MotionEvent.INVALID_POINTER_ID) { // if pointers are still valid
                        pointer1.pos = Vec2(
                            event.getX(event.findPointerIndex(pointer1.id)),
                            event.getY(event.findPointerIndex(pointer1.id))
                        )
                        pointer1.startingPos = Vec2(pointer1.pos.x, pointer1.pos.y)
                        pointer2.pos = Vec2(
                            event.getX(event.findPointerIndex(pointer2.id)),
                            event.getY(event.findPointerIndex(pointer2.id))
                        )
                        pointer2.startingPos = Vec2(pointer2.pos.x, pointer2.pos.y)
                    }
                }
            }
            MotionEvent.ACTION_POINTER_UP -> {
                val eventPointerId = event.getPointerId(event.actionIndex)
                if(pointer1.id == eventPointerId || pointer2.id == eventPointerId) {
                    if(pointer1.id == eventPointerId){
                        pointer1.id = MotionEvent.INVALID_POINTER_ID
                    } else pointer2.id = MotionEvent.INVALID_POINTER_ID

                    previousLifetimeRotation += currentGestureRotation
                    currentGestureRotation = 0f
                    if(previousLifetimeRotation > TWO_PI) {
                        previousLifetimeRotation -= TWO_PI
                    }
                    if(previousLifetimeRotation < TWO_PI) {
                        previousLifetimeRotation += TWO_PI
                    }
                }
            }
        }
    }
}