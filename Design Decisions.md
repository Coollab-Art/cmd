## Merging commands in the history

When you drag a slider, you don't want all of the intermediate values to be pushed to the history (usually). You would rather have a single commit that takes you from the value before dragging to the value where you released the slider.

As far as I know there are two approaches to achieve this: 
- Only push a command in the history when you are done manipulating your widget
- Send commands all the time, but make sure the history knows how to merge similar commands together

We chose the later for the following reasons:

### Only push a command in the history when you are done manipulating your widget

Pros:
- Easier to implement

Cons:
- All widgets have to store the value before and the value after, effectively doubling their size.

### Send commands all the time, but make sure the history knows how to merge similar commands together

Pros:
- Strongly exception safe, no risk of missing to send the "oh I finished" command that commits everything.
- Puts the burden on the library to handle merging, and frees the user from storing the values of widgets before edit. They only need to tell when they explicitly don't want a merge to happen (e.g. when you stop dragging a slider and then start dragging it again, we want this to be two separate commits).