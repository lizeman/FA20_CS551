## Special Behaivours for some edge case

### What if there are already 20 `compute` processes actively running?

For any futher `compute` process that tries to register, `manage` would not assign slot since there is no available slot.
Besides, `manage` would send an INTR signal to the new incoming `compute` process.
