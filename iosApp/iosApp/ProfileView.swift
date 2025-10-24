import SwiftUI

struct ProfileView: View {
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "person.fill")
                .font(.system(size: 64))
                .foregroundColor(.blue)

            Text("Profile Screen")
                .font(.largeTitle)
                .fontWeight(.bold)

            Text("Manage your account")
                .font(.body)
                .foregroundColor(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color(UIColor.systemBackground))
    }
}

struct ProfileView_Previews: PreviewProvider {
    static var previews: some View {
        ProfileView()
            .preferredColorScheme(.dark)
    }
}
