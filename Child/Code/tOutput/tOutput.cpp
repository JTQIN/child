/*************************************************************************\
**
**  tOutput.cpp: Functions for output classes tOutput and tLOutput
**
**  (see tOutput.h for a description of these classes)
**
**  $Id: tOutput.cpp,v 1.20 1999-04-01 16:03:04 gtucker Exp $
\*************************************************************************/

#include "tOutput.h"


/*************************************************************************\
**
**  Constructor
**
**  The constructor takes two arguments, a pointer to the grid mesh and
**  a reference to an open input file. It reads the base name for the
**  output files from the input file, and opens and initializes these.
**
**  Input: gridPtr -- pointer to a tGrid object (or descendant), assumed
**                    valid
**         infile -- reference to an open input file, assumed valid
**
\*************************************************************************/
template< class tSubNode >
tOutput<tSubNode>::tOutput( tGrid<tSubNode> * gridPtr, tInputFile &infile )
{
   assert( gridPtr > 0 );
   g = gridPtr;

   infile.ReadItem( baseName, "OUTFILENAME" );

   CreateAndOpenFile( &nodeofs, ".nodes" );
   CreateAndOpenFile( &edgofs, ".edges" );
   CreateAndOpenFile( &triofs, ".tri" );
   CreateAndOpenFile( &zofs, ".z" );
   
   
}


/*************************************************************************\
**
**  tOutput::CreateAndOpenFile
**
**  Opens the output file stream pointed to by theOFStream, giving it the
**  name <baseName><extension>, and checks to make sure that the ofstream
**  is valid.
**
**  Input:  theOFStream -- ptr to an ofstream object
**          extension -- file name extension (e.g., ".nodes")
**  Output: theOFStream is initialized to create an open output file
**  Assumes: extension is a null-terminated string, and the length of
**           baseName plus extension doesn't exceed kMaxNameSize+6
**           (ie, the extension is expected to be <= 6 characters)
**
\*************************************************************************/
template< class tSubNode >
void tOutput<tSubNode>::CreateAndOpenFile( ofstream *theOFStream,
                                           char *extension )
{
   char fullName[kMaxNameSize+6];  // name of file to be created
   
   strcpy( fullName, baseName );
   strcat( fullName, extension );
   theOFStream->open( fullName );

   if( !theOFStream->good() )
       ReportFatalError(
           "I can't create files for output. Storage space may be exhausted.");
        
}


/*************************************************************************\
**
**  tOutput::WriteOutput
**
**  This function writes information about the mesh to four files called
**  name.nodes, name.edges, name.tri, and name.z, where "name" is a
**  name that the user has specified in the input file and which is
**  stored in the data member baseName.
**
**  Input: time -- time of the current output time-slice
**  Output: the node, edge, and triangle ID numbers are modified so that
**          they are numbered according to their position on the list
**  Assumes: the four file ofstreams have been opened by the constructor
**           and are valid
**
**  TODO: deal with option for once-only printing of mesh when mesh not
**        deforming
\*************************************************************************/
template< class tSubNode >
void tOutput<tSubNode>::WriteOutput( double time )
{
   tGridListIter<tSubNode> niter( g->getNodeList() ); // node list iterator
   tGridListIter<tEdge> eiter( g->getEdgeList() );    // edge list iterator
   tListIter<tTriangle> titer( g->getTriList() );     // tri list iterator
   tNode * cn;       // current node
   tEdge * ce;       // current edge
   tTriangle * ct;   // current triangle
   int id;           // id of element (node, edge, or triangle)
   int nnodes = g->getNodeList()->getSize();  // # of nodes on list
   int nedges = g->getEdgeList()->getSize();  // "    edges "
   int ntri = g->getTriList()->getSize();     // "    triangles "

   cout << "tOutput::WriteOutput()\n";
   
   // Renumber IDs in order by position on list
   for( cn=niter.FirstP(), id=0; id<nnodes; cn=niter.NextP(), id++ )
       cn->setID( id );
   for( ce=eiter.FirstP(), id=0; id<nedges; ce=eiter.NextP(), id++ )
       ce->setID( id );
   for( ct=titer.FirstP(), id=0; id<ntri; ct=titer.NextP(), id++ )
       ct->setID( id );

   // Write node file and z file
   nodeofs << " " << time << endl << nnodes << endl;
   zofs << " " << time << endl << nnodes << endl;
   for( cn=niter.FirstP(); !(niter.AtEnd()); cn=niter.NextP() )
   {
      nodeofs << cn->getX() << " " << cn->getY() << " "
              << cn->getEdg()->getID() << " " << cn->getBoundaryFlag() << endl;
      zofs << cn->getZ() << endl;
   }
   
   // Write edge file
   edgofs << " " << time << endl << nedges << endl;
   for( ce=eiter.FirstP(); !(eiter.AtEnd()); ce=eiter.NextP() )
      edgofs << ce->getOriginPtrNC()->getID() << " "
             << ce->getDestinationPtrNC()->getID() << " "
             << ce->getCCWEdg()->getID() << endl;
   
   // Write triangle file
   int i;
   triofs << " " << time << endl << ntri << endl;
   for( ct=titer.FirstP(); !(titer.AtEnd()); ct=titer.NextP() )
   {
      for( i=0; i<=2; i++ )
          triofs << ct->pPtr(i)->getID() << " ";
      for( i=0; i<=2; i++ )
      {
          if( ct->tPtr(i) ) triofs << ct->tPtr(i)->getID() << " ";
          else triofs << "-1 ";
      }
      triofs << ct->ePtr(0)->getID() << " " 
             << ct->ePtr(1)->getID() << " " 
             << ct->ePtr(2)->getID() << endl;
   }

   // Call virtual function to write any additional data
   WriteNodeData( time );
   
}


/*************************************************************************\
**
**  tOutput::WriteNodeData
**
**  This is a virtual function which can be overridden to write any
**  additional node data. The base class version does nothing.
**
\*************************************************************************/
template< class tSubNode >
void tOutput<tSubNode>::WriteNodeData( double time ) 
{}


/*************************************************************************\
**
**  tLOutput constructor
**
**  Creates and opens a series of files for drainage areas, slopes, etc.
**
\*************************************************************************/
template< class tSubNode >
tLOutput<tSubNode>::tLOutput( tGrid<tSubNode> *g, tInputFile &infile ) 
        : tOutput<tSubNode>( g, infile )  // call base-class constructor
{
   CreateAndOpenFile( &drareaofs, ".area" );
   CreateAndOpenFile( &netofs, ".net" );
   CreateAndOpenFile( &slpofs, ".slp" );
   CreateAndOpenFile( &qofs, ".q" );
   CreateAndOpenFile( &layofs, ".lay" );
   CreateAndOpenFile( &texofs, ".tx" );
   
}


/*************************************************************************\
**
**  tLOutput::WriteNodeData
**
**  This overridden virtual function writes output for tLNodes, including
**  drainage areas, flow pathways, slopes, discharges, layer info, etc.
**
\*************************************************************************/
//TODO: should output boundary points as well so they'll map up with nodes
// for plotting. Means changing getSlope so it returns zero if flowedg
// undefined
template< class tSubNode >
void tLOutput<tSubNode>::WriteNodeData( double time )
{
   tGridListIter<tSubNode> ni( g->getNodeList() ); // node list iterator
   tSubNode *cn;   // current node
   int nActiveNodes = g->getNodeList()->getActiveSize(); // # active nodes
   int nnodes = g->getNodeList()->getSize();             // total # nodes
   int i, j;      // counters

   // Write current time in each file
   drareaofs << " " << time << "\n " << nActiveNodes << endl;
   netofs << " " << time << "\n " << nActiveNodes << endl;
   slpofs << " " << time << "\n " << nActiveNodes << endl;
   qofs << " " << time << "\n " << nnodes << endl;
   layofs << " " << time << "\n" << nActiveNodes << endl;
   texofs << " " << time << "\n" << nnodes << endl;

   // Write data, including layer info
   for( cn = ni.FirstP(); ni.IsActive(); cn = ni.NextP() )
   {
      assert( cn>0 );
      drareaofs << cn->getDrArea() << endl;
      if( cn->getDownstrmNbr() )
          netofs << cn->getDownstrmNbr()->getID() << endl;
      slpofs << cn->getSlope() << endl;
      layofs << cn->getNumLayer() << endl;
      i=0;
      while(i<cn->getNumLayer()){
         layofs << cn->getLayerCtime(i) << " " << cn->getLayerRtime(i) << " " << cn->getLayerEtime(i) << endl;
         layofs << cn->getLayerDepth(i) << " " << cn->getLayerErody(i) << " " << cn->getLayerSed(i) << endl;
         j=0;
         while(j<cn->getNumg()){
            layofs << cn->getLayerDgrade(i,j) << " ";
            j++;
         }
         layofs << endl;
         i++;
      }
   }

   // Write texture data
   for( cn = ni.FirstP(); !(ni.AtEnd()); cn = ni.NextP() ){
      qofs << cn->getQ() << endl;
      if( cn->getNumg()>1 ) // temporary hack TODO
      {
            texofs << cn->getLayerDgrade(0,0)/cn->getLayerDepth(0) << endl;
      }
      
   }
   
}

