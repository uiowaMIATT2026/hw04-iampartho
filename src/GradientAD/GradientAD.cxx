
// Usage:
//   GradientAD_iampartho \
//       --inputVolume  <input.nii.gz>  \
//       --outputVolume <output.nii.gz> \
//       --conductance  <K>             \
//       --timeStep     <dt>            \
//       --iterations   <N>
//

#include "GradientADCLP.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char * argv[])
{
  // ---------------------------------------------------------------------------
  // 1.  Parse arguments.
  //     Variables:  inputVolume, outputVolume, conductance, timeStep, iterations
  // ---------------------------------------------------------------------------
  PARSE_ARGS;

  // ---------------------------------------------------------------------------
  // 2.  Type aliases.
  //     GradientAnisotropicDiffusionImageFilter requires floating-point input.
  //     We cast short → float, filter, then cast back to short for output.
  // ---------------------------------------------------------------------------
  constexpr unsigned int Dimension = 3;
  using InputPixelType = short;
  using RealPixelType  = float;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using RealImageType  = itk::Image<RealPixelType,  Dimension>;

  // ---------------------------------------------------------------------------
  // 3.  Read and cast to float.
  // ---------------------------------------------------------------------------
  using ReaderType = itk::ImageFileReader<InputImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(inputVolume);

  using CastToRealType = itk::CastImageFilter<InputImageType, RealImageType>;
  auto castToReal = CastToRealType::New();
  castToReal->SetInput(reader->GetOutput());

  // ---------------------------------------------------------------------------
  // 4.  Apply gradient anisotropic diffusion.
  //
  //     SetConductanceParameter(K):  controls edge sensitivity.
  //       Low K → strongly preserves edges but less smoothing.
  //       High K → more smoothing, less edge preservation.
  //     SetTimeStep(dt):  numerical integration step. For 3D, dt <= 1/2^(dim+1) = 0.0625.
  //     SetNumberOfIterations(N):  each iteration diffuses the image one time step.
  // ---------------------------------------------------------------------------
  using DiffusionFilterType =
    itk::GradientAnisotropicDiffusionImageFilter<RealImageType, RealImageType>;

  auto diffusion = DiffusionFilterType::New();
  diffusion->SetInput(castToReal->GetOutput());
  diffusion->SetConductanceParameter(conductance);
  diffusion->SetTimeStep(timeStep);
  diffusion->SetNumberOfIterations(static_cast<unsigned int>(iterations));

  // Cast result back to short.
  using CastToShortType = itk::CastImageFilter<RealImageType, InputImageType>;
  auto castToShort = CastToShortType::New();
  castToShort->SetInput(diffusion->GetOutput());

  // ---------------------------------------------------------------------------
  // 5.  Write output.
  // ---------------------------------------------------------------------------
  using WriterType = itk::ImageFileWriter<InputImageType>;
  auto writer = WriterType::New();
  writer->SetFileName(outputVolume);
  writer->SetInput(castToShort->GetOutput());

  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cerr << "ERROR: " << err << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "GradientAD complete.\n"
            << "  Input       : " << inputVolume  << "\n"
            << "  Output      : " << outputVolume << "\n"
            << "  Conductance : " << conductance  << "\n"
            << "  Time Step   : " << timeStep     << "\n"
            << "  Iterations  : " << iterations   << std::endl;

  return EXIT_SUCCESS;
}
