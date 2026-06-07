# Logic Level

## Flag

`tribectf{3xpl0r1ng_th3_l0gic_lvl}`

## Solve

- Enter Solve instructions here
- Make solve scripts/writeup

## Solution Sketch
- The schematic seems to be enough for identifying each component, and what they're doing.
    - [Here's a list of common electronic symbols](https://en.wikipedia.org/wiki/Electronic_symbol)
- Once the components are identified, identify what the given arrangements of components mean. The circuit exclusively uses NPN transistor to create logic gates. Each single transistor in the first layer/column of the circuit acts as NOT gates. Second layer/column has NPN transistors connected in series representing AND gates. Rest of the layer/columns seem to have transistors in parallel representing OR gates. 
    - [Here's a link with logic gates created using NPN transistors](https://acastano.com/stem/computer-design/npn-pnp-logic-gates/)
    - [A video explaining how gates produce output](https://www.youtube.com/watch?v=OWlD7gL9gS0)
- Once the logic gates have been figured out, the output can be produced by plugging in the bits from `streams.csv`.