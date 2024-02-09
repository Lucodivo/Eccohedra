# Hilt

Constructors of Android app components (Activity, Fragment, ViewModel, etc.) are not in control of the app developer. Instead, instances of these classes are created by the Android OS at the request of the app developer. This makes it awkward to supply non-trivially serializable dependencies to Android app components.

[Hilt](https://developer.android.com/training/dependency-injection/hilt-android) is Google's recommended dependency injection (DI) solution for Android. Hilt helps solve the pain in passing dependencies into Android app components, while also making it simpler to provide fake dependencies for testing. It is an extension of Dagger2 modified specifically for easy integration with Android app components and acknowledging the lifecycles of those Android app components. 

Dagger can be difficult to learn all at once. And it is easy to forget elements of how it works, which is the reason for this documentation. Note that "Dagger", "Dagger2", and "Hilt" should be read as synonyms in this documentation. They are effectively the same for this documentation. That may not be the case elsewhere.

For myself (Connor), I find the hardest aspects to swallow are Hilt's heavy usage of annotations. I have found myself frustrated with my imperative code reading mind, thinking "How does Hilt know to do X??" To which the answer is usually that it knows how to do X through annotations. It achieves X through the use of the annotation processor and codegen. At the end of this document is a glossary of annotations that may be used in this project.

Major concepts in Dagger2 are [dagger modules](#dagger-modules) and [dagger components](#dagger-components).

Note: Due to the overloaded uses of the word "module"/"component", this documentation will always specify dagger components as "dagger components" and dagger modules as "dagger modules".

### Dagger Components

Dagger components are the classes that are actually responsible for injecting the dependencies. (Un)Fortunately, Hilt hides points of injection behind its codegen. Before Hilt, injecting Activities using dagger components might havbe looked something like this:

  ```
  class LoginActivity: Activity() {
      @Inject lateinit var loginViewModel: LoginViewModel

      override fun onCreate(savedInstanceState: Bundle?) {
          loginComponent = (application as MyDaggerApplication).appComponent.loginComponent()
          loginComponent.inject(this) // this component handles the injection of loginViewModel
          super.onCreate(savedInstanceState)
      }
  }

  ```
In the example above, the loginComponent is provided through the appComponent, which is attached to the Application instance. That login dagger component is used to inject the LoginViewModel. So, dagger components, in general, can be thought of as the injectors. The ones responsible for actually injecting dependencies. What these injectors are capable of injecting and how they instantiate those dependencies depends on which [dagger modules](#dagger-modules) are installed into the injector (dagger component).

With Hilt, the injection of Android app components happens behind the scenes. Injected member variables are available for use in Android app components after the following:
  - Application - super.onCreate()
  - Activity - super.onCreate()
  - ViewModel - After creation
  - Fragment - super.onAttach()
  - View - super()
  - Service = super.onCreate()

Hilt's default Android dagger components are nested inside of each other based on a [specified hierarchy](https://developer.android.com/training/dependency-injection/hilt-android#component-hierarchy). For instance, this hierarchy describes that a View that is annotated with *\@WithFragmentBindings* and exists within a Fragment will have a *\@ViewScoped* dagger component, that can also access the *\@FragmentScoped* dagger component of its encompassing Fragment, the *\@ActivityScoped* dagger component from the encompassing Activity, the *\@ActivityRetainedScoped* dagger component of that same Activity, and the application-wide *\@Singleton* dagger component.

Hilt is set up with an application-level dagger component in the Application class through the *\@HiltAndroidApp* annotation. This generated dagger component is attached to the Application object's lifecycle and provides dependencies to the Application object. It also is the root dagger component of the entire app, meaning that all other components can access the dependencies it provides (What dependencies does it provide??????????). Injection must propagate down through the Application to Activities and then from Activities into Fragments. Activities and Fragments must be annotated with *\@AndroidEntryPoint* if they rely on Hilt for DI. Activities must also be annotated with *\@AndroidEntryPoint* if it intends to spawn any Fragments that rely on Hilt for DI.

### Dagger Modules

Dagger modules are classes annotated with *\@Module*.
  ```
  @Module
  class DatabaseModule {
      \\ ...
  }
  ```
These modules contain *\@Provides* annotated methods which inform Hilt how to provide dependencies for [dagger components](#dagger-components). These methods may also be annotated with *\@Singleton* to signify that the provided dependency should be instantiated only once within a given dagger component.
  ```
  @Module
  class DatabaseModule {
      @Singleton
      @Provides
      fun provideAppDatabase(@ApplicationContext context: Context): AppDatabase {
          return AppDatabase.getInstance(context)
      }

      @Provides
      fun providePlantDao(appDatabase: AppDatabase): PlantDao {
          return appDatabase.plantDao()
      }
  }
  ```

Modules can also declare whether a dependency should be a singleton (*\@Singleton*) within the component it is installed in.

Hilt includes default dagger components that are tied to the lifecycles of Android app components:  
*ActivityComponent* - Scoped to the lifecycle of an Activity. Accessible by the Activity, and any View or Fragment instances spawned under that Activity.  
*ActivityRetainedComponent*  - Scoped to the lifecycle of the Activity (persisting through configurations). Accessible by the Activity, associated ViewModel, and any View or Fragment instances spawned under the Activity.  
*FragmentComponent* - Scoped to the lifecycle of the Fragment. Accessible by the Fragment and any View instances spawned under that Fragment that has been annotated with *\@WithFragmentBindings*.  
*ServiceComponent* - Scoped to the lifecycle of the Service. Accessible by the Service.   
*ViewComponent* - Scoped to the lifecycle of the View. Accessible by the View.  
*ViewWithFragmentComponent* - Scoped to the lifecycle of the View that has been annotated with *\@WithFragmentBindings*. Accessible by the View.
*ViewModelComponent* - Scoped to the lifecycle of the ViewModel. Accessible by the ViewModel.  
*SingletonComponent* - Scoped to the lifecycle of the Application. Accessible by the Application, any Activity, any Service, any Fragment, any ViewModel, and any View.  

Android Components:  
Application class MUST be annotated using *\@HiltAndroidApp* if Hilt is being used in the application. It hich creates the root dagger component that all other dagger components rely on.  
ViewModel class must be annotated using *\@HiltViewModel* if it has any field injection (*\@Inject*).  
Activity, Fragment, View, and Service classes must be annotated using *\@AndroidEntryPoint* if they have any field injection (*\@Inject*) or if they parent any Android app component instances that require Hilt's DI.  



## Glossary

### Annotation Glossary
*\@HiltAndroidApp* - Marking the Application class where Dagger components should be generated  
*\@HiltViewModel* - Marks a view model for injection by dagger (Note that *\@ViewModelInject* served a similar purpose but has since been deprecated) 
*\@AndroidEntryPoint* - Enables member injection for Activities, Fragments, Views, Services, and BroadcastReceivers  
*\@Singleton* - Signifies that the provided dependency should be treated as a Singleton within it's component  
*\@Provides* - Signifies that this method is responsible for providing the dependency of it's return type.  
*\@Binds* - A slightly more compact version of *\@Provides* that skips implementation. 
*\@Module* - Marks a class as a dagger module. Encapsulates one or many *\@Provides*/*\@Binds* methods, which are each responsible for informing dagger components how to provide specific dependencies.  
*\@Component* - Marks a class as an injector (dagger component), which bridges the *\@Module* to *\@Inject* fields.  
*\@InstallIn(XxxxxComponent::class)* - Used to link a module with a specific dagger component.  
*\@Inject* - Specifies that a member variable is a dependency that should be fulfilled by Hilt.  
*\@HiltAndroidTest* - Used for testing. Used to generate Hilt dagger components for each test. This will need to be used alongside a *HiltAndroidRule* to test the class.  
*\@UninstallModules(XxxxModule::class)* - Used for testing. Removes a module from installed components. This allows the test to provide its own modules with its own @Provides/@Binds methods, ultimately providing test specified dependency injection.  

*\@Binds* vs *\@Provides*
- Both *\@Binds* and *\@Provides* declare which type is being provided through their method return type.
- Both *\@Binds* and *\@Provides* declare their dependencies through method parameters
- Only a *\@Provides* method may have a custom implementation ofr exactly how the instance is provided through the method body. 
- A *\@Binds* method is an abstract method.
- Only a *\@Provides* method can have any number of parameters
- A *\@Binds* method may only have a single parameter, who's type is assignable as the return type. And who's dependencies are constructor injected.
- A *\@Provides* method may be a static method or an instance method. It is recommended that *\@Provides* methods be static where they can, as they do not require an instantiation of the Module.
- *\@Binds* methods and non-static *\@Provides* methods cannot go into the same dagger module, as *\@Binds* methods are not attached to any concrete class and instance *\@Provides* methods are.
- *\@Binds* may result in less codegen by avoiding a Provider\<T> for the concrete implementation of the class and only producing a Provider\<T> for the interface. Using *\@Provides* may produce both.

*\@EntryPoint* - An annotation added to an interface that describes the dependencies desired from a specific dagger component. We use @InstallIn(XxxxxComponent::class) to specify which component we want dependencies from. An example on how to use @EntryPoint is shown below:  
  ```
  class PlaylistContentProvider: ContentProvider() {
    @EntryPoint
    @InstallIn(ApplicationComponent::class)
    inferface PlaylistCPEntryPoint {
      fun getMusicDatabase(): MusicDatabase
    }

    override fun query(...) {
      val entryPoint = EntryPointAccessors.fromApplication(context, PlaylistCPEntryPoint::class.java)
      val database = entryPoint.getMusicDatabase()

      // ...
    }
  }
  ```

#### Scope Annotations
*\@XxxScoped* - Bindings should exist for the life of an X: 
- *\@ActivityScoped* - Activity  
- *\@ActivityRetainedScoped* - Activity (surviving configuration)  
- *\@FragmentScoped* - Fragment  
- *\@ServiceScoped* - Service  
- *\@ViewModelScoped* - ViewModel  
- *\@ViewScoped* - View  


#### Kotlin Annotations (NOT specific to Hilt) 
*\@Qualifier* - This is a meta-annotation. It is used to specify that the following annotation will be used as a qualifer to distinguish between two objects of the same type.
  - In Dagger, this is used to create and consume between two *\@Provides* methods that return the same interface/abstract class.  
  ```
  // NetworkModule.kt
  @Qualifier
  @Retention(AnnotationRetention.BINARY)
  annotation class AuthInterceptorOkHttpClient

  @Qualifier
  @Retention(AnnotationRetention.BINARY)
  annotation class OtherInterceptorOkHttpClient

  @Module
  @InstallIn(SingletonComponent::class)
  object NetworkModule {

    @AuthInterceptorOkHttpClient
    @Provides
    fun provideAuthInterceptorOkHttpClient(
      authInterceptor: AuthInterceptor
    ): OkHttpClient {
        return OkHttpClient.Builder()
                 .addInterceptor(authInterceptor)
                 .build()
    }

    @OtherInterceptorOkHttpClient
    @Provides
    fun provideOtherInterceptorOkHttpClient(
      otherInterceptor: OtherInterceptor
    ): OkHttpClient {
        return OkHttpClient.Builder()
                 .addInterceptor(otherInterceptor)
                 .build()
    }
  }

  // ExampleActivity.kt
  @AndroidEntryPoint
  class ExampleActivity: AppCompatActivity() {

    @AuthInterceptorOkHttpClient
    @Inject lateinit var okHttpClient: OkHttpClient
  }

  // ExampleServiceImpl.kt
  class ExampleServiceImpl @Inject constructor(
    @OtherInterceptorOkHttpClient private val okHttpClient: OkHttpClient
  )
  ```
*\@Retention* - This is a meta-annotation. It is used to specify the scope of the following annotation. 
  - There are three scopes:  
    - SOURCE - The annotation is only valid in compile time and is removed in the binary output.  
    - BINARY - The annotation persists in binary output but cannot be accessed via reflection.  
    - RUNTIME - The annotation persists in binary output and can be used via reflection. This is the default retention policy.  
  - *\@Retention* is used with the *\@Qualifier* annotation. Hilt does not require reflection, so a *\@Qualifier* annotation should always resemble the following:
    ```
    // NetworkModule.kt
    @Qualifier
    @Retention(AnnotationRetention.BINARY)
    annotation class ConcreteImplOfInterface
    ```
*\@ApplicationContext* - A predefined *\@Qualifier* for supplying ApplicationContext as the implementation for a Context dependency.  
*\@ActivityContext* - A predefined *\@Qualifier* for supplying ActivityContext as the implementation for a Context dependency.