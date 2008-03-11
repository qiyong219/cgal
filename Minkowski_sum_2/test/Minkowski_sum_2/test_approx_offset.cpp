#include <CGAL/basic.h>

#ifdef CGAL_USE_GMP
  // GMP is installed. Use the GMP rational number-type. 
  #include <CGAL/Gmpq.h>
  typedef CGAL::Gmpq                                    Rational;
#else
  // GMP is not installed. Use CGAL's exact rational number-type.
  #include <CGAL/MP_Float.h>
  #include <CGAL/Quotient.h>
  typedef CGAL::Quotient<CGAL::MP_Float>                Rational;
#endif

#include <CGAL/Cartesian.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/approximated_offset_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/Polygon_convex_decomposition_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include "read_polygon.h"
#include <list>
#include <cstring> // for std::strchr
#include <cstdio> // for std::sscanf


// instead of
//typedef CGAL::Cartesian<Rational>                   Kernel;
// workaround for VC++ 
struct Kernel : public CGAL::Cartesian<Rational> {};

typedef Kernel::Point_2                                 Point_2;
typedef CGAL::Polygon_2<Kernel>                         Polygon_2;

typedef CGAL::Gps_circle_segment_traits_2<Kernel>       Gps_traits_2;
typedef Gps_traits_2::Polygon_2                         Offset_polygon_2;
typedef Gps_traits_2::Polygon_with_holes_2   Offset_polygon_with_holes_2;

/*! Check if two polygons with holes are the same. */
bool are_equal (const Offset_polygon_with_holes_2& ph1,
                const Offset_polygon_with_holes_2& ph2)
{
  std::list<Offset_polygon_with_holes_2>   sym_diff;

  CGAL::symmetric_difference (ph1, ph2,
                              std::back_inserter(sym_diff));

  return (sym_diff.empty());
}

/*! The main program. */
int main (int argc, char **argv)
{
  // Read the input file.
  if (argc < 3)
  {
    std::cerr << "Usage: <polygon> <radius> ." << std::endl;
    return (1);
  }

  // Read the polygon from the input file.
  Polygon_2   pgn;
  
  if (! read_polygon (argv[1], pgn))
  {
    std::cerr << "Failed to read: <" << argv[1] << ">." << std::endl;
    return (1);
  }
  
  // Read the offset radius.
  int         numer, denom;

  if (std::sscanf (argv[2], "%d/%d", &numer, &denom) != 2)
  {
    std::cerr << "Invalid radius: " << argv[2] << std::endl;
    return (1);
  }

  Rational     r = Rational (numer, denom);
  const double eps = 0.0001;

  // Read the decomposition flags.
  bool         use_ssab = true;
  bool         use_opt = true;
  bool         use_hm = true;
  bool         use_greene = true;

  if (argc > 3)
  {
    use_ssab = (std::strchr (argv[3], 's') != NULL);
    use_opt = (std::strchr (argv[3], 'o') != NULL);
    use_hm = (std::strchr (argv[3], 'h') != NULL);
    use_greene = (std::strchr (argv[3], 'g') != NULL);
  }

  // Compute the Minkowski sum using the convolution method.
  Offset_polygon_with_holes_2                             offset_conv;

  std::cout << "Using the convolution method ... ";
  offset_conv = approximated_offset_2 (pgn, r, eps);
  std::cout << "Done." << std::endl;

  // Define auxiliary polygon-decomposition objects.
  CGAL::Small_side_angle_bisector_decomposition_2<Kernel> ssab_decomp;
  CGAL::Optimal_convex_decomposition_2<Kernel>            opt_decomp;
  CGAL::Hertel_Mehlhorn_convex_decomposition_2<Kernel>    hm_approx_decomp;
  CGAL::Greene_convex_decomposition_2<Kernel>             greene_decomp;
  Offset_polygon_with_holes_2                             offset_decomp;

  if (use_ssab)
  {
    std::cout << "Using the small-side angle-bisector decomposition ... ";
    offset_decomp = approximated_offset_2 (pgn, r, eps, ssab_decomp);
    if (are_equal (offset_conv, offset_decomp))
    {
      std::cout << "OK." << std::endl;
    }
    else
    {
      std::cout << "ERROR (different result)." << std::endl;
    }
  }

  if (use_opt)
  {
    std::cout << "Using the optimal convex decomposition ... ";
    offset_decomp = approximated_offset_2 (pgn, r, eps, opt_decomp);
    if (are_equal (offset_conv, offset_decomp))
    {
      std::cout << "OK." << std::endl;
    }
    else
    {
      std::cout << "ERROR (different result)." << std::endl;
    }
  }

  if (use_hm)
  {
    std::cout << "Using the Hertel--Mehlhorn decomposition ... ";
    offset_decomp = approximated_offset_2 (pgn, r, eps, hm_approx_decomp);
    if (are_equal (offset_conv, offset_decomp))
    {
      std::cout << "OK." << std::endl;
    }
    else
    {
      std::cout << "ERROR (different result)." << std::endl;
    }
  }

  if (use_greene)
  {
    std::cout << "Using the Greene decomposition ... ";
    offset_decomp = approximated_offset_2 (pgn, r, eps, greene_decomp);
    if (are_equal (offset_conv, offset_decomp))
    {
      std::cout << "OK." << std::endl;
    }
    else
    {
      std::cout << "ERROR (different result)." << std::endl;
    }
  }

  return (0);
}

