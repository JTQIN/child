#include <stdio.h>
#include <string.h>

#include "bmi_model_child.h"

#define VERBOSE (false)
#define CHILD_DEBUG

#if defined (CHILD_DEBUG)
# define CHECK_OR_THROW(assertion, err) { if (!(assertion)) throw err; }
#else
# define CHECK_OR_THROW(assertion, err) { }
#endif

void bmi::Model::Initialize (const char *file) {
  Child::Initialize (std::string (file));
  return;
}

void bmi::Model::Update () {
	if (initialized==false)
    throw bmi::CLASS_NOT_INITIALIZED;

  RunOneStorm ();
  MaskNodesBelowElevation (0);
}

void bmi::Model::UpdateUntil (double t) {
  double dt;

  dt = t - this->GetCurrentTime ();
  if (dt>0)
    Run (dt);
  MaskNodesBelowElevation (0);
}

void bmi::Model::Finalize () {
  this->CleanUp ();
}

void bmi::Model::GetComponentName (char * const name) {
  strcpy (name, "Child");
}

int bmi::Model::GetInputVarNameCount (void) {
  return this->input_var_name_count;
}

int bmi::Model::GetOutputVarNameCount (void) {
  return this->output_var_name_count;
}

void bmi::Model::GetInputVarNames (char * const * const names) {
  for (int i=0; i<this->input_var_name_count; i++) {
    strcpy (names[i], input_var_names[i]);
  }
}

void bmi::Model::GetOutputVarNames (char * const * const names) {
  for (int i=0; i<this->output_var_name_count; i++) {
    strcpy (names[i], output_var_names[i]);
  }
}

bmi::VarType bmi::Model::GetVarType (const char * var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  return bmi::VAR_TYPE_DOUBLE;
}

void bmi::Model::GetVarUnits (const char * var_name,
                              char * const units) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);
  strcpy (units, "m");
}

int bmi::Model::GetVarRank (const char * var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);
  return 2;
}

double bmi::Model::GetStartTime () {
  return 0.;
}

double bmi::Model::GetCurrentTime () {
  return this->time->getCurrentTime ();
}

double bmi::Model::GetEndTime () {
  return this->time->getCurrentTime () + this->time->RemainingTime ();
}

void bmi::Model::GetDouble(const char * var_name, double * const dest) {
  CHECK_OR_THROW(HasOutputVar (var_name), bmi::BAD_VAR_NAME);

  if (var_name && dest) {
    if (VERBOSE)
      std::cout << "childInterface::GetValueSet() here with request '"
        << var_name << "'\n";

    if (strcmp (var_name, "Elevation")==0 ||
        strcmp (var_name, "SeaFloorElevation")==0 ||
        strcmp (var_name, "CellElevation")==0) {
      CopyNodeElevations (dest);
    }
    else if (strcmp (var_name, "DepositionToElevation")==0 ||
             strcmp (var_name, "CellDepositionToElevation")==0) {
      CopyNodeElevations (dest);
    }
    else if (strcmp (var_name, "dz")==0 ||
             strcmp (var_name, "Erosion")==0 ||
             strcmp (var_name, "CellErosion")==0) {
      CopyNodeErosion (dest);
    }
    else if (strcmp (var_name, "WaterDischarge")==0 ||
             strcmp (var_name, "CellWaterDischarge")==0 ||
             strcmp (var_name, "water")==0) {
      CopyNodeDischarge (dest);
    }
    else if (strcmp (var_name, "BedLoadFlux")==0 ||
             strcmp (var_name, "CellBedLoadFlux")==0) {
      CopyNodeSedimentFlux (dest);
    }
    else {
      std::cerr << "Should not be reached!" << std::endl;
      throw bmi::BAD_VAR_NAME;
    }
  }
  return;
}

void bmi::Model::SetDouble (const char * var_name, double *vals) {
  if (VERBOSE)
    std::cout << "childInterface::set_double () here with request '"
                  << var_name << "'\n";

  //if (var_name.compare (0, 9, "Elevation")==0)
  if (strcmp (var_name, "Elevation")==0 ||
      strcmp (var_name, "SeaFloorElevation")==0 ||
      strcmp (var_name, "SeaFloorBasement")==0 ||
      strcmp (var_name, "Basement")==0)
  {
    return SetNodeElevations (vals);
  }
  else if (strcmp (var_name, "Uplift")==0) {
    return SetNodeUplift (vals);
  }
  else
  {
    std::cerr << "Warning: unrecognized value set '" << var_name << "'\n";
    std::cerr << "Request to set values ignored\n";
    throw bmi::BAD_VAR_NAME;
  }
}

int bmi::Model::GetVarPointCount (const char *var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  return mesh->getNodeList ()->getSize ();
}

int bmi::Model::GetVarCellCount (const char *var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (strncmp (var_name, "Cell", 4) == 0)
    return mesh->getTriList()->getSize();
  else
    return mesh->getNodeList ()->getSize ();
}

int bmi::Model::GetVarVertexCount (const char *var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (strncmp (var_name, "Cell", 4) == 0)
    return mesh->getTriList()->getSize()*3;
  else
    return mesh->getTriList()->getSize();
}

void bmi::Model::GetGridX (const char * var_name, double * const x) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (x && var_name) {
    const int n_nodes = mesh->getNodeList ()->getSize ();
    tMesh<tLNode>::nodeListIter_t ni( mesh->getNodeList() );

    for (tLNode* current_node=ni.FirstP(); !ni.AtEnd();
         current_node=ni.NextP())
    {
      x[current_node->getPermID ()] = current_node->getX ();

      if (VERBOSE)
        std::cout << "Node " << current_node->getPermID()
                  << " x=" << current_node->getX() << std::endl;
    }
  }
  else
    throw bmi::BAD_ARGUMENT;
   
  return;
}

void bmi::Model::GetGridY (const char * var_name, double * const y) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (y && var_name) {
    const int n_nodes = mesh->getNodeList ()->getSize ();
    tMesh<tLNode>::nodeListIter_t ni( mesh->getNodeList() );

    for (tLNode* current_node=ni.FirstP(); !ni.AtEnd();
         current_node=ni.NextP())
    {
      y[current_node->getPermID ()] = current_node->getY ();

      if (VERBOSE)
        std::cout << "Node " << current_node->getPermID()
                  << " y=" << current_node->getY() << std::endl;
    }
  }
  else
    throw bmi::BAD_ARGUMENT;
   
  return;
}

void bmi::Model::GetGridConnectivity (const char *var_name,
                                      int * connectivity) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (var_name && connectivity) {
    if (VERBOSE)
      std::cerr << "CHILD: var_name is " << var_name << std::endl;

    if (strcmp (var_name, "Cell") == 0) {
      int i=0;
      tMesh<tLNode>::triListIter_t ti (mesh->getTriList ());
      for (tTriangle * current_tri=ti.FirstP(); !ti.AtEnd();
           i+=3, current_tri=ti.NextP ()) {
        connectivity[i] = current_tri->pPtr (0)->getPermID ();
        connectivity[i+1] = current_tri->pPtr (1)->getPermID ();
        connectivity[i+2] = current_tri->pPtr (2)->getPermID ();
      }
    }
    else {
      int i=0;
      tMesh<tLNode>::nodeListIter_t ni(mesh->getNodeList ());

      for (tLNode * current_node=ni.FirstP(); !ni.AtEnd();
           i++, current_node=ni.NextP()) {
          connectivity[i] = current_node->getPermID ();
      }
    }
  }
  else
    throw bmi::BAD_ARGUMENT;

  return;
}

void bmi::Model::GetGridOffset (const char * var_name,
                                int * const offset) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);

  if (var_name && offset) {
    const int n_offsets = GetVarCellCount (var_name);
    if (strcmp (var_name, "Cell") == 0) {
      for (int i=0, offset_id=3; i<n_offsets; i++, offset_id+=3)
        offset[i] = offset_id;
    }
    else {
      for (int i=0, offset_id=1; i<n_offsets; i++, offset_id++)
        offset[i] = offset_id;
    }
  }

  return;
}

bmi::GridType bmi::Model::GetGridType (const char * var_name) {
  CHECK_OR_THROW(HasOutputVar (var_name) || HasInputVar (var_name), bmi::BAD_VAR_NAME);
  return bmi::GRID_TYPE_UNSTRUCTURED;
}

bool bmi::Model::HasInputVar (const char * var_name) {
  for (int i=0; i<this->input_var_name_count; i++) {
    if (strcmp (input_var_names[i], var_name) == 0)
      return true;
  }
  return false;
}

bool bmi::Model::HasOutputVar (const char * var_name) {
  for (int i=0; i<this->output_var_name_count; i++) {
    if (strcmp (output_var_names[i], var_name) == 0)
      return true;
  }
  return false;
}

void bmi::Model::SetInputVarNames (const char **names) {
  if (input_var_names) {
    for (int i=0; i<input_var_name_count; ++i)
      free (input_var_names[i]);
    delete input_var_names;
  }
  input_var_name_count = 0;

  if (names) {
    for (const char **name=names; *name; ++name)
      ++input_var_name_count;

    input_var_names = new char*[input_var_name_count];
    for (int i=0; i<input_var_name_count; ++i)
      input_var_names[i] = strdup (names[i]);
  }
  else
    input_var_names = NULL;
}

void bmi::Model::SetOutputVarNames (const char **names) {
  if (output_var_names) {
    for (int i=0; i<output_var_name_count; ++i)
      free (output_var_names[i]);
    delete output_var_names;
  }
  output_var_name_count = 0;

  if (names) {
    for (const char **name=names; *name; ++name)
      ++output_var_name_count;

    output_var_names = new char*[output_var_name_count];
    for (int i=0; i<output_var_name_count; ++i)
      output_var_names[i] = strdup (names[i]);
  }
  else
    output_var_names = NULL;
}
