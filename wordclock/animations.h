/*
 * animations.h
 *
 *  Created on: Oct 14, 2017
 *      Author: rutger.huijgen
 */
#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_

typedef enum {
    ANIMATION_NONE = 0,
    ANIMATION_TRANSITION,
    ANIMATION_ALL,
    ANIMATION_FIRE,
    ANIMATION_ALL_ON,
    ANIMATION_TETRIS,
    ANIMATION_MESSAGE
} EAnimationType;


// ANIMATIONS
void DoAnimation();
void ShowSplash();
void AnimationSetMessageText(char* txt);
char* AnimationGetMessageText();
void DisplayWord(char* str);
void Fire(uint32_t ms);
void DisplayGreeting();


#endif /* ANIMATIONS_H_ */
