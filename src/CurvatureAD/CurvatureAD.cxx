
// Usage:
//   CurvatureAD_iampartho \
//       --inputVolume  <input.nii.gz>  \
//       --outputVolume <output.nii.gz> \
//       --conductance  <K>             \
//       --timeStep     <dt>            \
//       --iterations   <N>
//

#include "CurvatureADCLP.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char * argv[])
{

  PARSE_ARGS;


  constexpr unsigned int Dimension = 3;
  using InputPixelType = short;
  using RealPixelType  = float;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using RealImageType  = itk::Image<RealPixelType,  Dimension>;


  using ReaderType = itk::ImageFileReader<InputImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(inputVolume);

  using CastToRealType = itk::CastImageFilter<InputImageType, RealImageType>;
  auto castToReal = CastToRealType::New();
  castToReal->SetInput(reader->GetOutput());


  using DiffusionFilterType =
    itk::CurvatureAnisotropicDiffusionImageFilter<RealImageType, RealImageType>;

  auto diffusion = DiffusionFilterType::New();
  diffusion->SetInput(castToReal->GetOutput());
  diffusion->SetConductanceParameter(conductance);
  diffusion->SetTimeStep(timeStep);
  diffusion->SetNumberOfIterations(static_cast<unsigned int>(iterations));

  // Cast result back to short.
  using CastToShortType = itk::CastImageFilter<RealImageType, InputImageType>;
  auto castToShort = CastToShortType::New();
  castToShort->SetInput(diffusion->GetOutput());


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

  std::cout << "CurvatureAD complete.\n"
            << "  Input       : " << inputVolume  << "\n"
            << "  Output      : " << outputVolume << "\n"
            << "  Conductance : " << conductance  << "\n"
            << "  Time Step   : " << timeStep     << "\n"
            << "  Iterations  : " << iterations   << std::endl;

  return EXIT_SUCCESS;
}
