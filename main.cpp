#include <iostream>
#include <random>
#include <vector>

//#include "libqhullcpp/RboxPoints.h"

// for nice cpp-style stuff
#include "libqhullcpp/QhullFacet.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullPoints.h"
#include "libqhullcpp/QhullVertex.h"
#include "libqhullcpp/QhullVertexSet.h"
#include "libqhullcpp/Qhull.h"

// for qhull C lib stuff not available in cpp stuff yet
#include "libqhull_r/libqhull_r.h"

int main(void) {
    QHULL_LIB_CHECK
     
    // qhull's realT type should be used in case it changes; but it's a double
    int dim = 4, npts = 300;
    std::vector< realT > pts(dim*npts);
    
    // randomize points
    std::uniform_real_distribution< realT > rand_uni_dist(0., 5.);
    std::random_device r;
    std::minstd_rand engine(r()); 
    auto generator = std::bind(rand_uni_dist, engine);
    std::generate(pts.begin(), pts.end(), generator);
    
   
    // "d" is qdelaunay command-line switch option. You may also need "Qt" to force simplices in the
    //   triangulation, but the triangulation won't be unique.
    orgQhull::Qhull q("", dim, npts, pts.data(), "d"); // "d Qt");
    orgQhull::QhullPoints vs = q.points(); // STL vector of QhullPoint, I think
    std::cout << " Number of points " << vs.size() << std::endl;
    orgQhull::QhullFacetList fs = q.facetList();
    std::cout << " Number of simplices : " << fs.size() << std::endl;

    // loop over facets and do stuff
    for (auto f: fs) {
        orgQhull::QhullVertexSet facet_vs = f.vertices();
        // std::cout << facet_vs;
        std::vector< orgQhull::QhullVertex > std_vs = facet_vs.toStdVector();
        //std::cout << " Facet has " << std_vs.size() << " vertices" << std::endl;
    }
    
    // Now find facet containing point
    std::vector< realT > interpolation_pt(dim + 1); //  plus 1 for lifting the point to paraboloid
    //   see http://www.qhull.org/html/qh-faq.htm#vclosest
    std::generate(interpolation_pt.begin(), interpolation_pt.begin() + dim, generator);
    for (int i = 0; i < dim; ++i)
        interpolation_pt[dim] += interpolation_pt[i]*interpolation_pt[i];

    // get qh pointer needed to call qhull_r C API stuff
    orgQhull::QhullQh *qh = q.qh();
    qh_setdelaunay(qh, dim + 1, 1, interpolation_pt.data());

    boolT isOutside;
    realT bestDistance;
    facetT *closest_facet;
    closest_facet = qh_findbestfacet(qh, interpolation_pt.data(), qh_ALL, &bestDistance, &isOutside);
    
    std::cout << " Point ";
    for (auto x: interpolation_pt)
        std::cout << x << " ";
    if (isOutside)
        std::cout << "is outside ";
    else
        std::cout << "is inside ";
    std::cout << " of and nearest to simplex " << closest_facet->id << std::endl;

    // make facet a nice CPP data struct to access it
    orgQhull::QhullFacet closest_f(q, closest_facet);
    std::vector< orgQhull::QhullVertex > vs_for_interpolation = closest_f.vertices().toStdVector();
    for (auto v: vs_for_interpolation)
        std::cout << v;

    return 0;
}
