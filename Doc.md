https://hackmd.io/@yFRZf2itQymgxXtGGDUi8g/ByRzKlyVn

* Structure:
    1. **main.cpp** -> main process, handle mouse and keyboard inputs, update graphics and physics.
    2. **ChaiWorld** class -> handles the initialization of world properties, include a singleton. **Use only this singleton**.
    3. object classes
        * **Rigid** class -> contain rigid body object and its properties.
        * **Deformable** class -> contain GEL object and its properties.
        * **Polygon** class -> attempts to use polygon objects to simulate deformable objects (in progress).
    4. **MultiCursor** class -> a self define cursor that can touch both deformable and rigid objects, the example cursor did not provide this functionality.
    5. Macro.h -> trivial stuff, just extract for convenience, can put some global variables into it.
* Process:
    1. add the objects you want to display in the scene under ```// COMPOSE THE VIRTUAL SCENE ```in main.cpp, refer to the objects there to initialize
    2. Check the object constructors to initial the properties, including **position**, **size**, and **coefficients**
    3. in main.cpp updateHaptics function, it will call the Chaiworld updateHapticsMulti() to update all objects status
    4. look for // update cGELSkeletonLink elongation, the code after this comment will update m_kSpringElongation in realtime
    5. the force response is also calculated in this function, modify the code to see the difference
* About "Data-Driven Elastic Models for Cloth: Modeling and Measurement":
    1. There is a lookup table under // update cGELSkeletonLink elongation designed for the data comes from this paper
    2. start from this basic hook law formula![](https://i.imgur.com/GDv6lda.png), since the experiment is on same plane as the cloth woven coordinates, the coefficient matrix can simplify to ![](https://i.imgur.com/YGz3GOe.png)
    3. piecewise linear elastic model formulates $C$ as a piecewise linear function $C(\epsilon)$ of strain tensor $\epsilon$ ![](https://i.imgur.com/KgmEPd6.png)
    4. $R_\phi$ is the rotational matrix defined by strain angle $\phi$, and another variable $\lambda_{max}$ is related to stress and strain tensor but not specify how to calculate in this paper. (https://www.continuummechanics.org/principalstrain.html)
    5. They use data points interpolation to get $C$. Each data point contains four parameters, c11, c12, c22 and c33 as used in Equation 2. $C(\lambda_{max}, \phi)$ is then efined by linearly interpolating data points over $\lambda_{max}$ and $\phi$, respectively.
    6. Here is the result that can be used in the project, once know how the coefficients are calculate, replace the lookup table. http://graphics.berkeley.edu/papers/Wang-DDE-2011-08/material_parameters.pdf
* Notice that sometimes the cursor will not appear in the scene, I suggests there are some initialization order problem. If encounter this error, just restart scene until you see the cursor.
