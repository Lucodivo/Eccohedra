## Scene List & Scene Potential Architecture
The app is fairly simple and, at least conceptually, should really only contain two screens:
1) The scene list
2) The scene
I've thought a lot about how to communicate between these two simple screens. The problem is simple,
the scene list must inform the scene "view" screen which scene to play. There are a few solutions
that I have considered.

### Potential Solution #1: SceneListActivity w/ XxxxxSceneActivity for Each Scene

Positives:
- The most straight forward implementation
- Debugging scene simply requires run configuration set to XxxxxSceneActivity

Concerns:
- Duplicate code that doesn't take advantage of the generic GLSurfaceView nor GLSurfaceView.Renderer interfaces
- Each new scene requires a new Scene implementation, editing the list of scenes in SceneListActivity,
  a new XxxxxSceneActivity (pollutes project with extra files), and edit of the AndroidManifest.xml (pollutes
  Manifest)

### Potential Solution #2: Parcelable w/ SceneListActivity & SceneActivity

Positives:
- SceneActivity can stay generic, not worrying about what Scene is coming in
- Each new scene would only require a new Scene implementation and an edit of the list of scenes
  in SceneListActivity.

Concerns:
- Every scene will have to implement Parcelable. Ugly, duplicate code, distracting.
- Editing run configurations to take in a Parcelable is terrible at best, impossible at worst.
  Debugging scenes could be done by just hardcoding specific scene in SceneActivity w/ run configuration
  set to SceneActivity.

### Potential Solution #2: Activity ViewModel w/ SceneList Fragment & Scene Fragment

Positives:
- Android ViewModels are already a pattern taken in this project.
    - Not only are they great for isolating the UI logic and bringing the project closer to something
      like MVVM, but they are absolutely essential in avoiding the nightmare that is configuration changes
      and recreating activities.
- Scenes can take advantage of generic implementations. SceneFragment needs know nothing about the
  implementation. It would just be grabbing some object that implements the interface from the Activity
  ViewModel.
- Each new scene would only require a new Scene implementation and an edit of the list of scenes
  in SceneListActivity.
- The two screens feel like they fit the core List-Detail mold of Fragments. It isn't only a standard
  to use Fragments for List-Detail experiences, but is more flexible when preparing the apps for larger
  screen sizes. And opens the door for showing the scene list while a scene is running.

Concerns:
- The ViewModel is used as the medium between two Fragments. This means that a single
  ViewModel is supplying the view state and interactions for two separate views/Fragments. This is less separation of these
  scenes than either of the other solutions. If we are to make an exception that these two views may share a single ViewModel,
  this method should have its limits. Tacking on additional views onto this ViewModel should be cautioned/avoided.
- Debugging: Editing run configurations to debug a scene would require debug code in the encompassing Activity.
  Not terrible but not as dead simple as having an Activity for each scene.