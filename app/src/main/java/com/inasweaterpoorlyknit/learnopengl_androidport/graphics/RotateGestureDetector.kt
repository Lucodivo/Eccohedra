package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.view.MotionEvent
import glm_.vec2.Vec2


class RotateGestureDetector {
    data class Pointer(
        var id: Int = MotionEvent.INVALID_POINTER_ID,
        var pos: Vec2 = Vec2(-1.0f, -1.0f),
        var startingPos: Vec2 = Vec2(-1.0f, -1.0f)
    )

    private val pointer1 = Pointer()
    private val pointer2 = Pointer()
    private var previousLifetimeRotation: Float = 0.0f // radians
    private var currentGestureRotation: Float = 0.0f
    val lifetimeRotation
        get() = previousLifetimeRotation + currentGestureRotation
    val isActive: Boolean
        get() = pointer1.id != MotionEvent.INVALID_POINTER_ID && pointer2.id != MotionEvent.INVALID_POINTER_ID
    private val centerFocus: Vec2
        get() = Vec2((pointer1.pos.x + pointer2.pos.x) * 0.5f, (pointer1.pos.y + pointer2.pos.y) * 0.5f)

    private fun updateTotalRotation() {
        currentGestureRotation = angleBetweenLines(pointer1.startingPos, pointer2.startingPos, pointer1.pos, pointer2.pos)
    }

    fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                pointer1.id = event.getPointerId(event.actionIndex);
                pointer1.startingPos.x = event.x
                pointer1.startingPos.y = event.y
                pointer1.pos.x = pointer1.startingPos.x
                pointer1.pos.y = pointer1.startingPos.y
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if (isActive) {
                    pointer1.pos.x = event.getX(event.findPointerIndex(pointer1.id))
                    pointer1.pos.y = event.getY(event.findPointerIndex(pointer1.id))
                    pointer2.pos.x = event.getX(event.findPointerIndex(pointer2.id))
                    pointer2.pos.y = event.getY(event.findPointerIndex(pointer2.id))
                    updateTotalRotation()
                }
                return true
            }
            MotionEvent.ACTION_UP -> {
                pointer1.id = MotionEvent.INVALID_POINTER_ID;
                return true
            }
            MotionEvent.ACTION_POINTER_DOWN -> {
                pointer2.id = event.getPointerId(event.actionIndex);
                pointer1.pos.x = event.getX(event.findPointerIndex(pointer1.id))
                pointer1.pos.y = event.getY(event.findPointerIndex(pointer1.id))
                pointer1.startingPos.x = pointer1.pos.x
                pointer1.startingPos.y = pointer1.pos.y
                pointer2.pos.x = event.getX(event.findPointerIndex(pointer2.id))
                pointer2.pos.y = event.getY(event.findPointerIndex(pointer2.id))
                pointer2.startingPos.x = pointer2.pos.x
                pointer2.startingPos.y = pointer2.pos.y
                return true
            }
            MotionEvent.ACTION_POINTER_UP -> {
                pointer2.id = MotionEvent.INVALID_POINTER_ID;
                previousLifetimeRotation += currentGestureRotation
                currentGestureRotation = 0.0f
                if(previousLifetimeRotation > TWO_PI) {
                    previousLifetimeRotation -= TWO_PI
                }
                if(previousLifetimeRotation < TWO_PI) {
                    previousLifetimeRotation += TWO_PI
                }
                return true
            }
            else -> return false
        }
    }
}