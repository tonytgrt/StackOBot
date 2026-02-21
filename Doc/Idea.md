Overview
---

- We already have a third person base project in Unreal where a player can use WASD to move, Space to jump, and Space again to active jetpack that allows it to hover and move in the air for a while.
- Player needs to reach a certain very high up altitude to win
- Disks with collision box will fall down from the sky with random speeds
- Player can aim and shoot the falling disks to freeze them
- Player has to jump onto the frozen platform to get higher to win

Implementation Details
---

- The aim and fire system will be like any third person shooting game where:
- RMB will slightly move the TPP cam from its original pos directly behind the player to the right, and center of screen will display a crossfire aimer
- LMB will fire a blue laser beam forward from the player. While holding RMB and aiming this should look like it's firing towards the crossfire. 
- Each disk has the same size. We bound their perimeters by squares and make them a grid in the XY plane of Unreal 5. At any given time in a cell of the grid there should always exist exactly 1 disk that is either falling with a -Z velocity or frozen by player. If a falling disk touches the ground it will respawn with a different random speed.
- Player can only freeze 1 disk above them. This frozen disk will turn red. If the player tries to freeze another disk above them the previously frozen red disk will restore its speed before it gets frozen.
- Player can freeze infinitely many disks below them which also includes the disk they are currently standing on. These frozen disks are green. A red disk will turn green if the player gets above it. If the player actively shoots a falling disk below them it will also turn green. If the player falls below any green disk it immediately loses the green frozen state and restores its previous speed. 
- Player may get hit by a falling disk from above and fall down as a result while they are airborne.