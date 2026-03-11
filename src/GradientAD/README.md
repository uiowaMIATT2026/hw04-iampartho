# GradientAD — Parameter Selection for Smoothing with Edge Preservation

## Overview

Gradient anisotropic diffusion (Perona-Malik diffusion) applies the PDE:

    ∂I/∂t = div[ g(|∇I|) ∇I ]

where g(·) = exp(−(|∇I|/K)²) is the conductance function. Large gradients
(edges) produce small g values, suppressing diffusion across edges while
allowing it in flat regions.

## Parameters

**Conductance (K):** Controls edge sensitivity.
- Low K (0.5–1.0): Only nearly-flat regions diffuse; edges are strongly preserved
  but noise in high-gradient areas remains.
- Moderate K (2.0–3.0): Good balance — smooth noise in tissue interiors while
  keeping major anatomical boundaries.
- High K (5.0+): Almost isotropic diffusion; edges are eroded similarly to a
  Gaussian blur.

**Time Step (dt):** Must satisfy dt ≤ 1/2^(D+1) = 0.0625 for 3D (D=3) to
ensure numerical stability. Using dt = 0.0625 is the recommended maximum.
Smaller dt requires more iterations for the same total diffusion.

**Iterations (N):** Each iteration advances the PDE by one time step.
Total "diffusion time" = N × dt.
- 5 iterations @ dt=0.0625 → mild smoothing
- 20 iterations @ dt=0.0625 → strong smoothing, edges still preserved at K=2.0
- 50+ iterations → very smooth interior regions, but thin edges may gradually erode

## Recommended Parameters

For a T1 MRI brain image (moderate noise):

    --conductance 2.0 --timeStep 0.0625 --iterations 10

This setting smoothly removes Gaussian noise in white matter and CSF while
keeping grey/white matter boundaries and cortical surfaces well-defined.

For a CT image (Hounsfield units, higher contrast):

    --conductance 3.0 --timeStep 0.0625 --iterations 15

Higher conductance is appropriate because CT edges are typically sharper and
higher in contrast than MRI soft-tissue edges.
